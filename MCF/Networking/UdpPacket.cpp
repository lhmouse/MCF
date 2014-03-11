// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UdpPacket.hpp"
#include "../../MCFCRT/c/ext/assert.h"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "_NetworkingUtils.hpp"
using namespace MCF;

// 嵌套类定义。
class UdpPacket::xDelegate : NO_COPY {
private:
	__MCF::WSAInitializer xm_vWSAInitializer;

	__MCF::SharedSocket xm_sockPeer;
	PeerInfo xm_vPeerInfo;

public:
	xDelegate(const __MCF::SharedSocket &sockPeer, const void *pSockAddr, std::size_t uSockAddrLen)
		: xm_sockPeer(sockPeer)
		, xm_vPeerInfo(pSockAddr, uSockAddrLen)
	{
	}
	xDelegate(const PeerInfo &vServerInfo) noexcept
		: xm_sockPeer()
		, xm_vPeerInfo(vServerInfo)
	{
	}

public:
	void Send(const VVector<std::uint8_t> &vecBuffer){
		if(!xm_sockPeer){
			const short shFamily = xm_vPeerInfo.IsIPv4() ? AF_INET : AF_INET6;

			__MCF::UniqueSocket sockServer(::socket(shFamily, SOCK_DGRAM, IPPROTO_UDP));
			if(!sockServer){
				MCF_THROW(::WSAGetLastError(), L"::socket() 失败。");
			}

			SOCKADDR_STORAGE vSockAddr;
			std::size_t uSockAddrLen;
			BZero(vSockAddr);
			vSockAddr.ss_family = shFamily;
			if(shFamily == AF_INET){
				auto &vSockAddrIn = reinterpret_cast<SOCKADDR_IN &>(vSockAddr);
				BCopy(vSockAddrIn.sin_addr, xm_vPeerInfo.m_au8IPv4);
				BCopy(vSockAddrIn.sin_port, xm_vPeerInfo.m_u16Port);
				uSockAddrLen = sizeof(SOCKADDR_IN);
			} else {
				auto &vSockAddrIn6 = reinterpret_cast<SOCKADDR_IN6 &>(vSockAddr);
				BCopy(vSockAddrIn6.sin6_addr, xm_vPeerInfo.m_au16IPv6);
				BCopy(vSockAddrIn6.sin6_port, xm_vPeerInfo.m_u16Port);
				uSockAddrLen = sizeof(SOCKADDR_IN6);
			}
			if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddr, uSockAddrLen)){
				MCF_THROW(::WSAGetLastError(), L"::connect() 失败。");
			}

			xm_sockPeer.Reset(sockServer.Release());
		}

		const int nBytesWritten = ::send(xm_sockPeer.Get(), (const char *)vecBuffer.GetData(), vecBuffer.GetSize(), 0);
		if(nBytesWritten < 0){
			MCF_THROW(::WSAGetLastError(), L"::send() 失败。");
		}
	}
	const PeerInfo &GetPeerInfo() const noexcept {
		return xm_vPeerInfo;
	}
};

// 构造函数和析构函数。
UdpPacket::UdpPacket(
	const void *ppServer,
	const void *pSockAddr,
	std::size_t uSockAddrLen,
	const void *pData,
	std::size_t uSize
)
	: xm_pDelegate(std::make_shared<xDelegate>(*(const __MCF::SharedSocket *)ppServer, pSockAddr, uSockAddrLen))
	, xm_vecBuffer((const std::uint8_t *)pData, uSize)
{
}

UdpPacket::UdpPacket() noexcept {
}
UdpPacket::UdpPacket(const PeerInfo &vServerInfo, const void *pData, std::size_t uSize)
	: xm_pDelegate(std::make_shared<xDelegate>(vServerInfo))
	, xm_vecBuffer((const std::uint8_t *)pData, uSize)
{
}
UdpPacket::~UdpPacket(){
}

// 其他非静态成员函数。
bool UdpPacket::IsValid() const noexcept {
	return (bool)xm_pDelegate;
}

void UdpPacket::Send() const {
	if(!xm_pDelegate){
		MCF_THROW(WSAENOTCONN, L"UdpPacket 无效。");
	}
	xm_pDelegate->Send(xm_vecBuffer);
}
const PeerInfo &UdpPacket::GetPeerInfo() const {
	if(!xm_pDelegate){
		MCF_THROW(WSAENOTCONN, L"UdpPacket 无效。");
	}
	return xm_pDelegate->GetPeerInfo();
}
