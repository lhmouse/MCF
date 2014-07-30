// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UdpSender.hpp"
#include "_SocketUtils.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
using namespace MCF;

namespace MCF {

namespace Impl {
	extern PeerInfo LocalInfoFromSocket(SOCKET sockLocal);
}

}

namespace {

class UdpSenderDelegate : CONCRETE(UdpSender) {
private:
	const Impl::WSAInitializer xm_vWSAInitializer;

	const Impl::SharedSocket xm_sockPeer;
	const PeerInfo xm_vPeerInfo;
	const PeerInfo xm_vLocalInfo;

	SOCKADDR_STORAGE xm_vSockAddr;
	int xm_nSockAddrSize;

public:
	UdpSenderDelegate(Impl::SharedSocket &&sockPeer, const void *pSockAddr, std::size_t uSockAddrSize)
		: xm_sockPeer	(std::move(sockPeer))
		, xm_vPeerInfo	(pSockAddr, uSockAddrSize)
		, xm_vLocalInfo	(Impl::LocalInfoFromSocket(xm_sockPeer.Get()))
	{
		std::memcpy(&xm_vSockAddr, pSockAddr, uSockAddrSize);
		xm_nSockAddrSize = (int)uSockAddrSize;
	}

public:
	const PeerInfo &GetPeerInfo() const noexcept {
		return xm_vPeerInfo;
	}
	const PeerInfo &GetLocalInfo() const noexcept {
		return xm_vLocalInfo;
	}

	void Send(const void *pData, std::size_t uSize){
		if(::sendto(
			xm_sockPeer.Get(), (const char *)pData, (int)uSize,
			0, (const SOCKADDR *)&xm_vSockAddr, xm_nSockAddrSize
		) < 0){
			MCF_THROW(::GetLastError(), L"::sendto() 失败。"_wso);
		}
	}
};

}

namespace MCF {

namespace Impl {
	std::unique_ptr<UdpSender> UdpSenderFromSocket(
		SharedSocket vSocket,
		const void *pSockAddr,
		std::size_t uSockAddrSize
	){
		return std::make_unique<UdpSenderDelegate>(std::move(vSocket), pSockAddr, uSockAddrSize);
	}
}

}

// 静态成员函数。
std::unique_ptr<UdpSender> UdpSender::Create(const PeerInfo &piServerInfo){
	Impl::WSAInitializer vWSAInitializer;

	const short shFamily = piServerInfo.IsIPv4() ? AF_INET : AF_INET6;
	Impl::SharedSocket sockServer(::socket(shFamily, SOCK_DGRAM, IPPROTO_UDP));
	if(!sockServer){
		MCF_THROW(::GetLastError(), L"::socket() 失败。"_wso);
	}

	SOCKADDR_STORAGE vSockAddr;
	const int nSockAddrSize = piServerInfo.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
	if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddr, nSockAddrSize)){
		MCF_THROW(::GetLastError(), L"::connect() 失败。"_wso);
	}
	return std::make_unique<UdpSenderDelegate>(std::move(sockServer), &vSockAddr, (std::size_t)nSockAddrSize);
}
std::unique_ptr<UdpSender> UdpSender::Create(const PeerInfo &piServerInfo, const PeerInfo &piLocalInfo){
	Impl::WSAInitializer vWSAInitializer;

	const short shFamily = piServerInfo.IsIPv4() ? AF_INET : AF_INET6;
	Impl::SharedSocket sockServer(::socket(shFamily, SOCK_DGRAM, IPPROTO_UDP));
	if(!sockServer){
		MCF_THROW(::GetLastError(), L"::socket() 失败。"_wso);
	}

	SOCKADDR_STORAGE vLocalAddr;
	const int nLocalAddrSize = piLocalInfo.ToSockAddr(&vLocalAddr, sizeof(vLocalAddr));
	if(::bind(sockServer.Get(), (const SOCKADDR *)&vLocalAddr, nLocalAddrSize)){
		MCF_THROW(::GetLastError(), L"::bind() 失败。"_wso);
	}

	SOCKADDR_STORAGE vSockAddr;
	const int nSockAddrSize = piServerInfo.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
	if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddr, nSockAddrSize)){
		MCF_THROW(::GetLastError(), L"::connect() 失败。"_wso);
	}
	return std::make_unique<UdpSenderDelegate>(std::move(sockServer), &vSockAddr, (std::size_t)nSockAddrSize);
}

std::unique_ptr<UdpSender> UdpSender::CreateNoThrow(const PeerInfo &piServerInfo){
	Impl::WSAInitializer vWSAInitializer;

	const short shFamily = piServerInfo.IsIPv4() ? AF_INET : AF_INET6;
	Impl::SharedSocket sockServer(::socket(shFamily, SOCK_DGRAM, IPPROTO_UDP));
	if(!sockServer){
		return nullptr;
	}

	SOCKADDR_STORAGE vSockAddr;
	const int nSockAddrSize = piServerInfo.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
	if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddr, nSockAddrSize)){
		return nullptr;
	}
	return std::make_unique<UdpSenderDelegate>(std::move(sockServer), &vSockAddr, (std::size_t)nSockAddrSize);
}
std::unique_ptr<UdpSender> UdpSender::CreateNoThrow(const PeerInfo &piServerInfo, const PeerInfo &piLocalInfo){
	Impl::WSAInitializer vWSAInitializer;

	const short shFamily = piServerInfo.IsIPv4() ? AF_INET : AF_INET6;
	Impl::SharedSocket sockServer(::socket(shFamily, SOCK_DGRAM, IPPROTO_UDP));
	if(!sockServer){
		return nullptr;
	}

	SOCKADDR_STORAGE vLocalAddr;
	const int nLocalAddrSize = piLocalInfo.ToSockAddr(&vLocalAddr, sizeof(vLocalAddr));
	if(::bind(sockServer.Get(), (const SOCKADDR *)&vLocalAddr, nLocalAddrSize)){
		return nullptr;
	}

	SOCKADDR_STORAGE vSockAddr;
	const int nSockAddrSize = piServerInfo.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
	if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddr, nSockAddrSize)){
		return nullptr;
	}
	return std::make_unique<UdpSenderDelegate>(std::move(sockServer), &vSockAddr, (std::size_t)nSockAddrSize);
}

// 其他非静态成员函数。
const PeerInfo &UdpSender::GetPeerInfo() const noexcept {
	ASSERT(dynamic_cast<const UdpSenderDelegate *>(this));

	return ((const UdpSenderDelegate *)this)->GetPeerInfo();
}
const PeerInfo &UdpSender::GetLocalInfo() const noexcept {
	ASSERT(dynamic_cast<const UdpSenderDelegate *>(this));

	return ((const UdpSenderDelegate *)this)->GetLocalInfo();
}

void UdpSender::Send(const void *pData, std::size_t uSize){
	ASSERT(dynamic_cast<UdpSenderDelegate *>(this));

	((UdpSenderDelegate *)this)->Send(pData, uSize);
}
