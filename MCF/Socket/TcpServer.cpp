// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TcpServer.hpp"
#include "TcpPeer.hpp"
#include "_SocketUtils.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

namespace MCF {

namespace Impl {
	extern std::unique_ptr<TcpPeer> TcpPeerFromSocket(
		UniqueSocket vSocket,
		const void *pSockAddr,
		std::size_t uSockAddrSize
	);
}

}

namespace {

const unsigned long TRUE_VALUE	= TRUE;
const unsigned long FALSE_VALUE	= FALSE;

class TcpServerDelegate : CONCRETE(TcpServer) {
private:
	Impl::WSAInitializer xm_vWSAInitializer;

	Impl::UniqueSocket xm_sockListen;
	volatile bool xm_bStopNow;

public:
	TcpServerDelegate(
		const PeerInfo &piBoundOnto,
		bool bExclusive,
		bool bReuseAddr
	)
		: xm_bStopNow(false)
	{
		const short shFamily = piBoundOnto.IsIPv4() ? AF_INET : AF_INET6;
		xm_sockListen.Reset(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
		if(!xm_sockListen){
			MCF_THROW(::GetLastError(), L"::socket() 失败。"_wso);
		}

		if((shFamily == AF_INET6) && ::setsockopt(xm_sockListen.Get(), IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&FALSE_VALUE, sizeof(FALSE_VALUE))){
			MCF_THROW(::GetLastError(), L"::setsockopt() 失败。"_wso);
		}
		if(bExclusive && ::setsockopt(xm_sockListen.Get(), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char *)&TRUE_VALUE, sizeof(TRUE_VALUE))){
			MCF_THROW(::GetLastError(), L"::setsockopt() 失败。"_wso);
		}
		if(bReuseAddr && ::setsockopt(xm_sockListen.Get(), SOL_SOCKET, SO_REUSEADDR, (const char *)&TRUE_VALUE, sizeof(TRUE_VALUE))){
			MCF_THROW(::GetLastError(), L"::setsockopt() 失败。"_wso);
		}
		if(::ioctlsocket(xm_sockListen.Get(), (long)FIONBIO, (unsigned long *)&TRUE_VALUE)){
			MCF_THROW(::GetLastError(), L"::ioctlsocket() 失败。"_wso);
		}
		SOCKADDR_STORAGE vSockAddr;
		const int nSockAddrSize = piBoundOnto.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
		if(::bind(xm_sockListen.Get(), (const SOCKADDR *)&vSockAddr, nSockAddrSize)){
			MCF_THROW(::GetLastError(), L"::bind() 失败。"_wso);
		}

		if(::listen(xm_sockListen.Get(), SOMAXCONN)){
			MCF_THROW(::GetLastError(), L"::listen() 失败。"_wso);
		}
	}

public:
	std::unique_ptr<TcpPeer> GetClientTimeout(unsigned long long ullMilliSeconds){
		std::unique_ptr<TcpPeer> pPeer;

		unsigned long ulNextSleepTime = 0;
		WaitUntil<0x100>(
			[&](unsigned long ulInterval){
				if(__atomic_load_n(&xm_bStopNow, __ATOMIC_ACQUIRE)){
					return true;
				}
				if(ulNextSleepTime == 0){
					ulNextSleepTime = 1;
				} else {
					ulNextSleepTime = Min(ulNextSleepTime, ulInterval);
					::Sleep(ulNextSleepTime);

					ulNextSleepTime <<= 1;
				}

				SOCKADDR_STORAGE vSockAddr;
				int nSockAddrSize = sizeof(vSockAddr);
				Impl::UniqueSocket sockClient(::accept(xm_sockListen.Get(), (SOCKADDR *)&vSockAddr, &nSockAddrSize));
				if(!sockClient){
					return false;
				}
				if(::ioctlsocket(sockClient.Get(), (long)FIONBIO, (unsigned long *)&FALSE_VALUE)){
					MCF_THROW(::GetLastError(), L"::ioctlsocket() 失败。"_wso);
				}
				pPeer = Impl::TcpPeerFromSocket(std::move(sockClient), &vSockAddr, (std::size_t)nSockAddrSize);
				return true;
			},
			ullMilliSeconds
		);

		return std::move(pPeer);
	}
	void Shutdown(){
		__atomic_store_n(&xm_bStopNow, false, __ATOMIC_RELEASE);
	}
};

}

// 静态成员函数。
std::unique_ptr<TcpServer> TcpServer::Create(
	const PeerInfo &piBoundOnto,
	bool bExclusive,
	bool bReuseAddr
){
	return std::make_unique<TcpServerDelegate>(piBoundOnto, bExclusive, bReuseAddr);
}

// 其他非静态成员函数。
std::unique_ptr<TcpPeer> TcpServer::GetPeerTimeout(unsigned long long ullMilliSeconds){
	ASSERT(dynamic_cast<TcpServerDelegate *>(this));

	return static_cast<TcpServerDelegate *>(this)->GetClientTimeout(ullMilliSeconds);
}
std::unique_ptr<TcpPeer> TcpServer::GetPeer(){
	return GetPeerTimeout(WAIT_FOREVER);
}
void TcpServer::Shutdown(){
	ASSERT(dynamic_cast<TcpServerDelegate *>(this));

	return static_cast<TcpServerDelegate *>(this)->Shutdown();
}
