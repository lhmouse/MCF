// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TcpServer.hpp"
#include "TcpPeer.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "../Thread/Thread.hpp"
#include "../Thread/CriticalSection.hpp"
#include "../Thread/ConditionVariable.hpp"
#include "_SocketUtils.hpp"
#include <deque>
using namespace MCF;

namespace MCF {

namespace Impl {
	extern std::unique_ptr<TcpPeer> TcpPeerFromSocket(
		Impl::UniqueSocket vSocket,
		const void *pSockAddr,
		std::size_t uSockAddrSize
	);
}

}

namespace {

struct ClientInfo {
	Impl::UniqueSocket sockClient;
	SOCKADDR_STORAGE vSockAddr;
	int nSockAddrSize;
};

class TcpServerDelegate : CONCRETE(TcpServer) {
private:
	Impl::WSAInitializer xm_vWSAInitializer;

	Impl::UniqueSocket xm_sockListen;
	volatile bool xm_bStopNow;
	std::shared_ptr<Thread> xm_pthrdListener;

	const std::unique_ptr<CriticalSection> xm_pcsQueueLock;
	const std::unique_ptr<ConditionVariable> xm_pcondPeersAvailable;
	std::deque<ClientInfo> xm_deqClients;

public:
	explicit TcpServerDelegate(const PeerInfo &vBoundOnto)
		: xm_bStopNow				(false)
		, xm_pcsQueueLock			(CriticalSection::Create())
		, xm_pcondPeersAvailable	(ConditionVariable::Create())
	{
		static const DWORD TRUE_VALUE	= 1;
		static const DWORD FALSE_VALUE	= 0;

		const short shFamily = vBoundOnto.IsIPv4() ? AF_INET : AF_INET6;

		xm_sockListen.Reset(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
		if(!xm_sockListen){
			MCF_THROW(::GetLastError(), L"::socket() 失败。");
		}
		if(::setsockopt(xm_sockListen.Get(), SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (const char *)&TRUE_VALUE, sizeof(TRUE_VALUE))){
			MCF_THROW(::GetLastError(), L"::setsockopt() 失败。");
		}
		if(shFamily == AF_INET6){
			if(::setsockopt(xm_sockListen.Get(), IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&FALSE_VALUE, sizeof(FALSE_VALUE))){
				MCF_THROW(::GetLastError(), L"::setsockopt() 失败。");
			}
		}

		unsigned long ulTrueValue = 1;
		if(::ioctlsocket(xm_sockListen.Get(), (long)FIONBIO, &ulTrueValue)){
			MCF_THROW(::GetLastError(), L"::ioctlsocket() 失败。");
		}

		SOCKADDR_STORAGE vSockAddr;
		const int nSockAddrSize = vBoundOnto.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
		if(::bind(xm_sockListen.Get(), (const SOCKADDR *)&vSockAddr, nSockAddrSize)){
			MCF_THROW(::GetLastError(), L"::bind() 失败。");
		}

		if(::listen(xm_sockListen.Get(), SOMAXCONN)){
			MCF_THROW(::GetLastError(), L"::listen() 失败。");
		}

		xm_pthrdListener = Thread::Create(std::bind(&TcpServerDelegate::xListenerProc, this), false);
	}
	~TcpServerDelegate() noexcept {
		__atomic_store_n(&xm_bStopNow, true, __ATOMIC_RELEASE);
		xm_pthrdListener->Wait();
	}

private:
	void xListenerProc() noexcept {
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

		unsigned long ulSleepTime = 0;
		while(!__atomic_load_n(&xm_bStopNow, __ATOMIC_ACQUIRE)){
			try {
				if(ulSleepTime == 0){
					ulSleepTime = 1;
				} else {
					::Sleep(ulSleepTime);
					if(ulSleepTime < 0x100){
						ulSleepTime <<= 1;
					}
				}

				ClientInfo vClient;
				vClient.nSockAddrSize = sizeof(vClient.vSockAddr);
				vClient.sockClient.Reset(::accept(
					xm_sockListen.Get(),
					(SOCKADDR *)&vClient.vSockAddr,
					&(vClient.nSockAddrSize)
				));
				if(!vClient.sockClient){
					continue;
				}
				unsigned long ulFalseValue = 0;
				if(::ioctlsocket(vClient.sockClient.Get(), (long)FIONBIO, &ulFalseValue)){
					continue;
				}

				{
					const auto vLock = xm_pcsQueueLock->GetLock();
					xm_deqClients.emplace_back(std::move(vClient));
					xm_pcondPeersAvailable->Signal();
				}
				ulSleepTime = 0;
			} catch(...){
			}
		}

		{
			const auto vLock = xm_pcsQueueLock->GetLock();
			xm_pcondPeersAvailable->Broadcast();
		}
	}

public:
	ClientInfo GetClientTimeout(unsigned long ulMilliSeconds) noexcept {
		ClientInfo vClient;
		if(ulMilliSeconds == INFINITE){
			auto vLock = xm_pcsQueueLock->GetLock();
			for(;;){
				if(!xm_deqClients.empty()){
					vClient = std::move(xm_deqClients.front());
					xm_deqClients.pop_front();
					break;
				}
				if(__atomic_load_n(&xm_bStopNow, __ATOMIC_ACQUIRE)){
					break;
				}

				xm_pcondPeersAvailable->Wait(vLock);
			}
		} else {
			const auto ulWaitUntil = ::GetTickCount() + ulMilliSeconds;
			auto vLock = xm_pcsQueueLock->GetLock();
			for(;;){
				if(!xm_deqClients.empty()){
					vClient = std::move(xm_deqClients.front());
					xm_deqClients.pop_front();
					break;
				}
				if(__atomic_load_n(&xm_bStopNow, __ATOMIC_ACQUIRE)){
					break;
				}

				const auto ulCurrent = ::GetTickCount();
				if(ulWaitUntil <= ulCurrent){
					break;
				}
				if(!xm_pcondPeersAvailable->WaitTimeout(vLock, ulWaitUntil - ulCurrent)){
					break;
				}
			}
		}
		return std::move(vClient);
	}
};

}

// 静态成员函数。
std::unique_ptr<TcpServer> TcpServer::Create(const PeerInfo &vBoundOnto){
	return std::make_unique<TcpServerDelegate>(vBoundOnto);
}

// 其他非静态成员函数。
std::unique_ptr<TcpPeer> TcpServer::GetPeerTimeout(unsigned long ulMilliSeconds) noexcept {
	ASSERT(dynamic_cast<TcpServerDelegate *>(this));

	auto vClient = ((TcpServerDelegate *)this)->GetClientTimeout(ulMilliSeconds);
	if(!vClient.sockClient){
		return std::unique_ptr<TcpPeer>();
	}
	return Impl::TcpPeerFromSocket(
		std::move(vClient.sockClient),
		&(vClient.vSockAddr),
		(std::size_t)vClient.nSockAddrSize
	);
}
std::unique_ptr<TcpPeer> TcpServer::GetPeer() noexcept {
	return GetPeerTimeout(INFINITE);
}
