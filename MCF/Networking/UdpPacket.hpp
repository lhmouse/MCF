// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UDP_PACKET_HPP__
#define __MCF_UDP_PACKET_HPP__

#include "../../MCFCRT/cpp/ext/vvector.hpp"
#include "PeerInfo.hpp"
#include <memory>
#include <cstddef>
#include <cstdint>

namespace MCF {

class UdpServer;

class UdpPacket {
	friend class UdpServer;

private:
	class xDelegate;

private:
	std::shared_ptr<xDelegate> xm_pDelegate;
	VVector<std::uint8_t> xm_vecBuffer;

private:
	UdpPacket(
		const void *ppServer,
		const void *pSockAddr,
		std::size_t uSockAddrLen,
		const void *pData,
		std::size_t uSize
	);

public:
	UdpPacket() noexcept;
	UdpPacket(const PeerInfo &vServerInfo, const void *pData = nullptr, std::size_t uSize = 0);
	~UdpPacket();

public:
	bool IsValid() const noexcept;

	void Send() const;
	const PeerInfo &GetPeerInfo() const;

	const VVector<std::uint8_t> &GetBuffer() const noexcept {
		return xm_vecBuffer;
	}
	VVector<std::uint8_t> &GetBuffer() noexcept {
		return xm_vecBuffer;
	}

public:
	explicit operator bool() const noexcept {
		return IsValid();
	}
};

}

#endif
