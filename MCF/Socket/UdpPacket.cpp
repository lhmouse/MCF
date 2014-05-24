// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UdpPacket.hpp"
#include "_SocketUtils.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/VVector.hpp"
using namespace MCF;

// 其他非静态成员函数。
void UdpPacket::Send(bool bClearBuffer) const {
	Impl::WSAInitializer vWSAInitializer;

	const short shFamily = m_vPeerInfo.IsIPv4() ? AF_INET : AF_INET6;

	Impl::UniqueSocket sockPeer(::socket(shFamily, SOCK_DGRAM, IPPROTO_UDP));
	if(!sockPeer){
		MCF_THROW(::GetLastError(), L"::socket() 失败。");
	}

	VVector<unsigned char, 512u> vecTemp;
	vecTemp.Reserve(m_sbufData.GetSize());
	m_sbufData.Traverse(
		[&](auto pbyData, auto uSize){
			vecTemp.CopyToEnd(pbyData, uSize);
		}
	);

	SOCKADDR_STORAGE vSockAddr;
	const int nSockAddrSize = m_vPeerInfo.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
	if(::sendto(sockPeer.Get(), (const char *)vecTemp.GetData(), (int)vecTemp.GetSize(), 0, (const SOCKADDR *)&vSockAddr, nSockAddrSize) < 0){
		MCF_THROW(::GetLastError(), L"::sendto() 失败。");
	}

	if(bClearBuffer){
		m_sbufData.Clear();
	}
}
