// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TcpServer.hpp"
#include "TcpPeer.hpp"
#include "../Core/Thread.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/Event.hpp"
#include "../Core/CriticalSection.hpp"
#include "_SocketUtils.hpp"
#include <deque>
using namespace MCF;

// 嵌套类定义。
class TcpServer::xDelegate : NO_COPY {
private:
	enum class State {
		STOPPED,
		RUNNING,
		STOPPING
	};

private:
	__MCF::WSAInitializer xm_vWSAInitializer;

	volatile State xm_eState;
	__MCF::UniqueSocket xm_sockListen;

	CriticalSection xm_csQueueLock;
	std::deque<TcpPeer> xm_deqPeers;
	Event xm_evnPeersAvailable;
	Thread xm_thrdListener;

public:
	xDelegate()
		: xm_eState(State::STOPPED)
		, xm_evnPeersAvailable(true)
	{
	}
	~xDelegate(){
	}

private:
	void xListenerProc() noexcept {
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

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
				__MCF::UniqueSocket sockClient(::accept(xm_sockListen.Get(), (SOCKADDR *)&vSockAddr, &nSockAddrSize));
				if(!sockClient){
					continue;
				}
				unsigned long ulFalseValue = 0;
				if(::ioctlsocket(sockClient.Get(), FIONBIO, &ulFalseValue)){
					continue;
				}

				TcpPeer vPeer(&sockClient, &vSockAddr, nSockAddrSize);
				CRITICAL_SECTION_SCOPE(xm_csQueueLock){
					xm_deqPeers.emplace_back(std::move(vPeer));
					xm_evnPeersAvailable.Set();
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
		static const DWORD TRUE_VALUE	= 1;
		static const DWORD FALSE_VALUE	= 0;

		Stop();

		const short shFamily = vBoundOnto.IsIPv4() ? AF_INET : AF_INET6;

		xm_sockListen.Reset(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
		if(!xm_sockListen){
			MCF_THROW(::WSAGetLastError(), L"::socket() 失败。");
		}
		if(::setsockopt(xm_sockListen.Get(), SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (const char *)&TRUE_VALUE, sizeof(TRUE_VALUE))){
			MCF_THROW(::WSAGetLastError(), L"::setsockopt() 失败。");
		}
		if(shFamily == AF_INET6){
			if(::setsockopt(xm_sockListen.Get(), IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&FALSE_VALUE, sizeof(FALSE_VALUE))){
				MCF_THROW(::WSAGetLastError(), L"::setsockopt() 失败。");
			}
		}

		unsigned long ulTrueValue = 1;
		if(::ioctlsocket(xm_sockListen.Get(), FIONBIO, &ulTrueValue)){
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
		if(::bind(xm_sockListen.Get(), (const SOCKADDR *)&vSockAddr, sizeof(vSockAddr))){
			MCF_THROW(::WSAGetLastError(), L"::bind() 失败。");
		}

		if(::listen(xm_sockListen.Get(), SOMAXCONN)){
			MCF_THROW(::WSAGetLastError(), L"::listen() 失败。");
		}

		xm_evnPeersAvailable.Clear();
		xm_thrdListener.Start(std::bind(&xDelegate::xListenerProc, this));

		__atomic_store_n(&xm_eState, State::RUNNING, __ATOMIC_RELEASE);
	}
	void Stop() noexcept {
		if(__atomic_load_n(&xm_eState, __ATOMIC_ACQUIRE) != State::STOPPED){
			__atomic_store_n(&xm_eState, State::STOPPING, __ATOMIC_RELEASE);

			xm_thrdListener.JoinDetach();
			CRITICAL_SECTION_SCOPE(xm_csQueueLock){
				xm_deqPeers.clear();
			}
			xm_evnPeersAvailable.Set();
			xm_sockListen.Reset();

			__atomic_store_n(&xm_eState, State::STOPPED, __ATOMIC_RELEASE);
		}
	}

	TcpPeer GetPeerTimeout(unsigned long ulMilliSeconds) noexcept {
		TcpPeer vPeer;
		const auto ulWaitUntil = ::GetTickCount() + ulMilliSeconds;
		unsigned long ulTimeToWait = ulMilliSeconds;
		for(;;){
			if(!xm_evnPeersAvailable.WaitTimeout(ulTimeToWait)){
				break;
			}
			CRITICAL_SECTION_SCOPE(xm_csQueueLock){
				switch(xm_deqPeers.size()){
				case 1:
					xm_evnPeersAvailable.Clear();
				default:
					vPeer = std::move(xm_deqPeers.front());
					xm_deqPeers.pop_front();
				case 0:
					break;
				}
			}
			if(vPeer){
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
		return std::move(vPeer);
	}
};

// 构造函数和析构函数。
TcpServer::TcpServer()
	: xm_pDelegate(new xDelegate)
{
}
TcpServer::TcpServer(TcpServer &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
TcpServer &TcpServer::operator=(TcpServer &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
TcpServer::~TcpServer(){
}

// 其他非静态成员函数。
bool TcpServer::IsRunning() const noexcept {
	return xm_pDelegate->IsRunning();
}
void TcpServer::Start(const PeerInfo &vBoundOnto){
	xm_pDelegate->Start(vBoundOnto);
}
void TcpServer::Stop() noexcept {
	xm_pDelegate->Stop();
}

TcpPeer TcpServer::GetPeerTimeout(unsigned long ulMilliSeconds) noexcept {
	return xm_pDelegate->GetPeerTimeout(ulMilliSeconds);
}
TcpPeer TcpServer::GetPeer() noexcept {
	return xm_pDelegate->GetPeerTimeout(INFINITE);
}
