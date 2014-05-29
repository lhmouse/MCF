// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_UDP_PACKET_HPP_
#define MCF_UDP_PACKET_HPP_

#include "PeerInfo.hpp"
#include "../Core/VVector.hpp"
#include <memory>

namespace MCF {

class UdpPacket {
public:
	PeerInfo m_vPeerInfo;
	VVector<unsigned char, 508u> m_vecData;

public:
	explicit UdpPacket(const PeerInfo &vPeerInfo) noexcept
		: m_vPeerInfo(vPeerInfo)
	{
	}
	UdpPacket(const PeerInfo &vPeerInfo, const void *pData, std::size_t uSize) noexcept
		: m_vPeerInfo	(vPeerInfo)
		, m_vecData		((const unsigned char *)pData, uSize)
	{
	}
	template<std::size_t VECTOR_CAP>
	UdpPacket(const PeerInfo &vPeerInfo, VVector<unsigned char, VECTOR_CAP> vecData) noexcept
		: m_vPeerInfo	(vPeerInfo)
		, m_vecData		(std::move(vecData))
	{
	}

public:
	void Send() const;
};

}

#endif
