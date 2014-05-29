// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UdpPacket.hpp"
#include "_SocketUtils.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
using namespace MCF;

// 其他非静态成员函数。
void UdpPacket::Send() const {
	Impl::WSAInitializer vWSAInitializer;

	const short shFamily = m_vPeerInfo.IsIPv4() ? AF_INET : AF_INET6;

	Impl::UniqueSocket sockPeer(::socket(shFamily, SOCK_DGRAM, IPPROTO_UDP));
	if(!sockPeer){
		MCF_THROW(::GetLastError(), L"::socket() 失败。");
	}

	SOCKADDR_STORAGE vSockAddr;
	const int nSockAddrSize = m_vPeerInfo.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
	if(::sendto(sockPeer.Get(), (const char *)m_vecData.GetData(), (int)m_vecData.GetSize(), 0, (const SOCKADDR *)&vSockAddr, nSockAddrSize) < 0){
		MCF_THROW(::GetLastError(), L"::sendto() 失败。");
	}
}
