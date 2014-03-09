// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TcpServer.hpp"
#include "TcpPeer.hpp"
#include "../Core/Thread.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/Semaphore.hpp"
#include "../Core/CriticalSection.hpp"
#include "_NetworkingUtils.hpp"
#include <queue>
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
	__MCF::WSAInitializer xm_vInitializer;

	State xm_eState;
	__MCF::UniqueSocket xm_sockListen;

	CriticalSection xm_csQueueLock;
	std::queue<TcpPeer> xm_quePeers;
	Semaphore xm_semPeersAvailable;
	Thread xm_thrdListener;

public:
	xDelegate()
		: xm_eState(State::STOPPED)
		, xm_semPeersAvailable(0, LONG_MAX)
	{
	}
	~xDelegate(){
	}

private:
	void xListenerProc() noexcept {
		unsigned long ulSleepTime = 0;
		while(xm_eState == State::RUNNING){
			try {
				if(ulSleepTime == 0){
					ulSleepTime = 1;
				} else {
					if(ulSleepTime < 0x40){
						ulSleepTime <<= 1;
					}
					::Sleep(ulSleepTime);
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

				TcpPeer vPeer;
				vPeer.xAssign(&sockClient, &vSockAddr, nSockAddrSize);
				CRITICAL_SECTION_SCOPE(xm_csQueueLock){
					xm_quePeers.emplace(std::move(vPeer));
				}
				xm_semPeersAvailable.Signal();
				ulSleepTime = 0;
			} catch(...){
			}
		}
	}

public:
	bool IsRunning() const noexcept {
		return xm_eState != State::STOPPED;
	}
	void Start(const PeerInfo &vBoundOnto){
		Stop();

		const short shFamily = vBoundOnto.IsIPv4() ? AF_INET : AF_INET6;

		xm_sockListen.Reset(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
		if(!xm_sockListen){
			MCF_THROW(::WSAGetLastError(), L"::socket() 失败。");
		}
		if(shFamily == AF_INET6){
			const DWORD dwFalseValue = 0;
			if(::setsockopt(xm_sockListen.Get(), IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&dwFalseValue, sizeof(dwFalseValue))){
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

		xm_thrdListener.Start(std::bind(&xDelegate::xListenerProc, this));

		xm_eState = State::RUNNING;
	}
	void Stop() noexcept {
		if(xm_eState != State::STOPPED){
			xm_eState = State::STOPPING;

			xm_thrdListener.JoinDetach();
			xm_sockListen.Reset();

			xm_eState = State::STOPPED;
		}
	}

	TcpPeer GetPeerTimeout(unsigned long ulMilliSeconds) noexcept {
		TcpPeer vPeer;
		if(xm_semPeersAvailable.WaitTimeout(ulMilliSeconds)){
			CRITICAL_SECTION_SCOPE(xm_csQueueLock){
				ASSERT(!xm_quePeers.empty());

				vPeer = std::move(xm_quePeers.front());
				xm_quePeers.pop();
			}
		}
		return std::move(vPeer);
	}
};

// 构造函数和析构函数。
TcpServer::TcpServer()
	: xm_pDelegate(new xDelegate)
{
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
