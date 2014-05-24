// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_UDP_PACKET_HPP_
#define MCF_UDP_PACKET_HPP_

#include "PeerInfo.hpp"
#include "../Core/StreamBuffer.hpp"
#include <memory>

namespace MCF {

class UdpPacket {
public:
	PeerInfo m_vPeerInfo;
	StreamBuffer m_sbufData;

public:
	explicit UdpPacket(const PeerInfo &vPeerInfo) noexcept
		: m_vPeerInfo(vPeerInfo)
	{
	}
	UdpPacket(const PeerInfo &vPeerInfo, const void *pData, std::size_t uSize) noexcept
		: m_vPeerInfo	(vPeerInfo)
		, m_sbufData	(pData, uSize)
	{
	}
	UdpPacket(const PeerInfo &vPeerInfo, StreamBuffer sbufData) noexcept
		: m_vPeerInfo	(vPeerInfo)
		, m_sbufData	(std::move(sbufData))
	{
	}

public:
	void Send(bool bClearBuffer = true) const;
};

}

#endif
