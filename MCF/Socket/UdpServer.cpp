// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UdpServer.hpp"
#include "UdpPacket.hpp"
#include "PeerInfo.hpp"
#include "_SocketUtils.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

namespace {

class UdpServerDelegate : CONCRETE(UdpServer) {
private:
	Impl::WSAInitializer xm_vWSAInitializer;

	Impl::UniqueSocket xm_sockPeer;
	volatile bool xm_bStopNow;

public:
	UdpServerDelegate(
		const PeerInfo &piBoundOnto,
		bool bExclusive,
		bool bReuseAddr
	)
		: xm_bStopNow(false)
	{
		static const unsigned long TRUE_VALUE	= TRUE;
		static const unsigned long FALSE_VALUE	= FALSE;

		const short shFamily = piBoundOnto.IsIPv4() ? AF_INET : AF_INET6;

		xm_sockPeer.Reset(::socket(shFamily, SOCK_DGRAM, IPPROTO_UDP));
		if(!xm_sockPeer){
			MCF_THROW(::GetLastError(), L"::socket() 失败。");
		}
		if((shFamily == AF_INET6) && ::setsockopt(xm_sockPeer.Get(), IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&FALSE_VALUE, sizeof(FALSE_VALUE))){
			MCF_THROW(::GetLastError(), L"::setsockopt() 失败。");
		}
		if(bExclusive && ::setsockopt(xm_sockPeer.Get(), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char *)&TRUE_VALUE, sizeof(TRUE_VALUE))){
			MCF_THROW(::GetLastError(), L"::setsockopt() 失败。");
		}
		if(bReuseAddr && ::setsockopt(xm_sockPeer.Get(), SOL_SOCKET, SO_REUSEADDR, (const char *)&TRUE_VALUE, sizeof(TRUE_VALUE))){
			MCF_THROW(::GetLastError(), L"::setsockopt() 失败。");
		}

		if(::ioctlsocket(xm_sockPeer.Get(), (long)FIONBIO, (unsigned long *)&TRUE_VALUE)){
			MCF_THROW(::GetLastError(), L"::ioctlsocket() 失败。");
		}

		SOCKADDR_STORAGE vSockAddr;
		const int nSockAddrSize = piBoundOnto.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
		if(::bind(xm_sockPeer.Get(), (const SOCKADDR *)&vSockAddr, nSockAddrSize)){
			MCF_THROW(::GetLastError(), L"::bind() 失败。");
		}
	}

public:
	std::unique_ptr<UdpPacket> GetPacketTimeout(unsigned long long ullMilliSeconds){
		std::unique_ptr<UdpPacket> pPacket;

		unsigned long ulNextSleepTime = 0;
		WaitUntil<0x100>(
			[&](unsigned long ulInterval){
				if(__atomic_load_n(&xm_bStopNow, __ATOMIC_ACQUIRE)){
					return true;
				}
				if(ulNextSleepTime == 0){
					ulNextSleepTime = 1;
				} else {
					ulNextSleepTime = std::min(ulNextSleepTime, ulInterval);
					::Sleep(ulNextSleepTime);

					ulNextSleepTime <<= 1;
				}

				std::size_t uSize = 0x10000;
				VVector<unsigned char> vecTemp(uSize);
				SOCKADDR_STORAGE vSockAddr;
				int nSockAddrSize = (int)sizeof(vSockAddr);
				const int nBytesRead = ::recvfrom(xm_sockPeer.Get(), (char *)vecTemp.GetData(), (int)uSize, 0, (SOCKADDR *)&vSockAddr, &nSockAddrSize);
				if(nBytesRead < 0){
					const auto ulErrorCode = ::GetLastError();
					if(ulErrorCode != WSAEWOULDBLOCK){
						MCF_THROW(ulErrorCode, L"::recvfrom() 失败。");
					}
					return false;
				}

				pPacket = std::make_unique<UdpPacket>(
					PeerInfo(&vSockAddr, (std::size_t)nSockAddrSize),
					vecTemp.GetData(),
					(std::size_t)nBytesRead
				);
				return true;
			},
			ullMilliSeconds
		);

		return std::move(pPacket);
	}
	void Shutdown(){
		__atomic_store_n(&xm_bStopNow, false, __ATOMIC_RELEASE);
	}

	void JoinGroup(const PeerInfo &piGroup){
		SOCKADDR_STORAGE vSockAddr;
		piGroup.ToSockAddr(&(vSockAddr), sizeof(vSockAddr));

		int nResult;
		if(piGroup.IsIPv4()){
			struct ip_mreq vIPMreq;
			BCopy(vIPMreq.imr_multiaddr, reinterpret_cast<const SOCKADDR_IN &>(vSockAddr).sin_addr);
			BZero(vIPMreq.imr_interface);
			nResult = ::setsockopt(xm_sockPeer.Get(), IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&vIPMreq, sizeof(vIPMreq));
		} else {
			struct ipv6_mreq vIPv6Mreq;
			BCopy(vIPv6Mreq.ipv6mr_multiaddr, reinterpret_cast<const SOCKADDR_IN6 &>(vSockAddr).sin6_addr);
			vIPv6Mreq.ipv6mr_interface = 0;
			nResult = ::setsockopt(xm_sockPeer.Get(), IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (const char *)&vIPv6Mreq, sizeof(vIPv6Mreq));
		}
		if(nResult){
			MCF_THROW(::GetLastError(), L"::setsockopt() 失败。");
		}
	}
	void LeaveGroup(const PeerInfo &piGroup) noexcept {
		SOCKADDR_STORAGE vSockAddr;
		piGroup.ToSockAddr(&(vSockAddr), sizeof(vSockAddr));

		if(piGroup.IsIPv4()){
			struct ip_mreq vIPMreq;
			BCopy(vIPMreq.imr_multiaddr, reinterpret_cast<const SOCKADDR_IN &>(vSockAddr).sin_addr);
			BZero(vIPMreq.imr_interface);
			::setsockopt(xm_sockPeer.Get(), IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *)&vIPMreq, sizeof(vIPMreq));
		} else {
			struct ipv6_mreq vIPv6Mreq;
			BCopy(vIPv6Mreq.ipv6mr_multiaddr, reinterpret_cast<const SOCKADDR_IN6 &>(vSockAddr).sin6_addr);
			vIPv6Mreq.ipv6mr_interface = 0;
			::setsockopt(xm_sockPeer.Get(), IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (const char *)&vIPv6Mreq, sizeof(vIPv6Mreq));
		}
	}
};

}

// 静态成员函数。
std::unique_ptr<UdpServer> UdpServer::Create(
	const PeerInfo &piBoundOnto,
	bool bExclusive,
	bool bReuseAddr
){
	return std::make_unique<UdpServerDelegate>(piBoundOnto, bExclusive, bReuseAddr);
}

// 其他非静态成员函数。
std::unique_ptr<UdpPacket> UdpServer::GetPacketTimeout(unsigned long long ullMilliSeconds){
	ASSERT(dynamic_cast<UdpServerDelegate *>(this));

	return static_cast<UdpServerDelegate *>(this)->GetPacketTimeout(ullMilliSeconds);
}

void UdpServer::JoinGroup(const PeerInfo &piGroup){
	ASSERT(dynamic_cast<UdpServerDelegate *>(this));

	return static_cast<UdpServerDelegate *>(this)->JoinGroup(piGroup);
}
void UdpServer::LeaveGroup(const PeerInfo &piGroup) noexcept {
	ASSERT(dynamic_cast<UdpServerDelegate *>(this));

	return static_cast<UdpServerDelegate *>(this)->LeaveGroup(piGroup);
}
