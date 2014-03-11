// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UdpServer.hpp"
#include "UdpPacket.hpp"
#include "../Core/Thread.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/Event.hpp"
#include "../Core/CriticalSection.hpp"
#include "_SocketUtils.hpp"
#include <deque>
using namespace MCF;

// 嵌套类定义。
class UdpServer::xDelegate : NO_COPY {
private:
	enum class State {
		STOPPED,
		RUNNING,
		STOPPING
	};

private:
	__MCF::WSAInitializer xm_vWSAInitializer;

	volatile State xm_eState;
	__MCF::SharedSocket xm_sockServer;

	CriticalSection xm_csQueueLock;
	std::deque<UdpPacket> xm_deqPackets;
	Event xm_evnPacketsAvailable;
	Thread xm_thrdWorker;

public:
	xDelegate()
		: xm_eState(State::STOPPED)
		, xm_evnPacketsAvailable(true)
	{
	}
	~xDelegate(){
	}

private:
	void xWorkerProc() noexcept {
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

		enum : std::size_t {
			BUFFER_SIZE = 0x10000
		};
		const std::unique_ptr<char[]> pBuffer(new char[BUFFER_SIZE]);

		unsigned long ulSleepTime = 0;
		while(__atomic_load_n(&xm_eState, __ATOMIC_ACQUIRE) == State::RUNNING){
			try {
				if(ulSleepTime == 0){
					ulSleepTime = 1;
				} else {
					::Sleep(ulSleepTime);
					if(ulSleepTime < 0x20){
						ulSleepTime <<= 1;
					}
				}

				SOCKADDR_STORAGE vSockAddr;
				int nSockAddrSize = sizeof(vSockAddr);
				const int nBytesReceived = ::recvfrom(xm_sockServer.Get(), pBuffer.get(), BUFFER_SIZE, 0, (SOCKADDR *)&vSockAddr, &nSockAddrSize);
				if(nBytesReceived < 0){
					continue;
				}

				UdpPacket vPacket(&xm_sockServer, &vSockAddr, nSockAddrSize, pBuffer.get(), nBytesReceived);
				CRITICAL_SECTION_SCOPE(xm_csQueueLock){
					xm_deqPackets.emplace_back(std::move(vPacket));
					xm_evnPacketsAvailable.Set();
				}
				ulSleepTime = 0;
			} catch(...){
			}
		}
	}

public:
	bool IsRunning() const noexcept {
		return __atomic_load_n(&xm_eState, __ATOMIC_ACQUIRE) != State::STOPPED;
	}
	void Start(const PeerInfo &vBoundOnto){
		static const DWORD FALSE_VALUE	= 0;

		Stop();

		const short shFamily = vBoundOnto.IsIPv4() ? AF_INET : AF_INET6;

		xm_sockServer.Reset(::socket(shFamily, SOCK_DGRAM, IPPROTO_UDP));
		if(!xm_sockServer){
			MCF_THROW(::WSAGetLastError(), L"::socket() 失败。");
		}
		if(shFamily == AF_INET6){
			if(::setsockopt(xm_sockServer.Get(), IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&FALSE_VALUE, sizeof(FALSE_VALUE))){
				MCF_THROW(::WSAGetLastError(), L"::setsockopt() 失败。");
			}
		}

		unsigned long ulTrueValue = 1;
		if(::ioctlsocket(xm_sockServer.Get(), FIONBIO, &ulTrueValue)){
			MCF_THROW(::WSAGetLastError(), L"::ioctlsocket() 失败。");
		}

		SOCKADDR_STORAGE vSockAddr;
		BZero(vSockAddr);
		vSockAddr.ss_family = shFamily;
		if(shFamily == AF_INET){
			auto &vSockAddrIn = reinterpret_cast<SOCKADDR_IN &>(vSockAddr);
			BCopy(vSockAddrIn.sin_addr, vBoundOnto.m_au8IPv4);
			BCopy(vSockAddrIn.sin_port, vBoundOnto.m_u16Port);
		} else {
			auto &vSockAddrIn6 = reinterpret_cast<SOCKADDR_IN6 &>(vSockAddr);
			BCopy(vSockAddrIn6.sin6_addr, vBoundOnto.m_au16IPv6);
			BCopy(vSockAddrIn6.sin6_port, vBoundOnto.m_u16Port);
		}
		if(::bind(xm_sockServer.Get(), (const SOCKADDR *)&vSockAddr, sizeof(vSockAddr))){
			MCF_THROW(::WSAGetLastError(), L"::bind() 失败。");
		}

		xm_evnPacketsAvailable.Clear();
		xm_thrdWorker.Start(std::bind(&xDelegate::xWorkerProc, this));

		__atomic_store_n(&xm_eState, State::RUNNING, __ATOMIC_RELEASE);
	}
	void Stop() noexcept {
		if(__atomic_load_n(&xm_eState, __ATOMIC_ACQUIRE) != State::STOPPED){
			__atomic_store_n(&xm_eState, State::STOPPING, __ATOMIC_RELEASE);

			xm_thrdWorker.JoinDetach();
			CRITICAL_SECTION_SCOPE(xm_csQueueLock){
				xm_deqPackets.clear();
			}
			xm_evnPacketsAvailable.Set();
			xm_sockServer.Reset();

			__atomic_store_n(&xm_eState, State::STOPPED, __ATOMIC_RELEASE);
		}
	}

	UdpPacket GetPacketTimeout(unsigned long ulMilliSeconds) noexcept {
		UdpPacket vPacket;
		const auto ulWaitUntil = ::GetTickCount() + ulMilliSeconds;
		unsigned long ulTimeToWait = ulMilliSeconds;
		for(;;){
			if(!xm_evnPacketsAvailable.WaitTimeout(ulTimeToWait)){
				break;
			}
			CRITICAL_SECTION_SCOPE(xm_csQueueLock){
				switch(xm_deqPackets.size()){
				case 1:
					xm_evnPacketsAvailable.Clear();
				default:
					vPacket = std::move(xm_deqPackets.front());
					xm_deqPackets.pop_front();
				case 0:
					break;
				}
			}
			if(vPacket){
				break;
			}
			if(__atomic_load_n(&xm_eState, __ATOMIC_ACQUIRE) != State::RUNNING){
				break;
			}
			const auto ulCurrent = ::GetTickCount();
			if(ulWaitUntil <= ulCurrent){
				break;
			}
			ulTimeToWait = ulWaitUntil - ulCurrent;
		}
		return std::move(vPacket);
	}
};

// 构造函数和析构函数。
UdpServer::UdpServer()
	: xm_pDelegate(new xDelegate)
{
}
UdpServer::UdpServer(UdpServer &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
UdpServer &UdpServer::operator=(UdpServer &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
UdpServer::~UdpServer(){
}

// 其他非静态成员函数。
bool UdpServer::IsRunning() const noexcept {
	return xm_pDelegate->IsRunning();
}
void UdpServer::Start(const PeerInfo &vBoundOnto){
	xm_pDelegate->Start(vBoundOnto);
}
void UdpServer::Stop() noexcept {
	xm_pDelegate->Stop();
}

UdpPacket UdpServer::GetPacketTimeout(unsigned long ulMilliSeconds) noexcept {
	return xm_pDelegate->GetPacketTimeout(ulMilliSeconds);
}
UdpPacket UdpServer::GetPacket() noexcept {
	return xm_pDelegate->GetPacketTimeout(INFINITE);
}
