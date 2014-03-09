// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_PEER_INFO_HPP__
#define __MCF_PEER_INFO_HPP__

#include "../../MCF/Core/Utilities.hpp"
#include <cstdint>

namespace MCF {

class PeerInfo {
public:
	union {
		std::uint16_t m_au16IPv6[8];
		struct {
			std::uint16_t m_au16IPv4Prefix[6];
			std::uint8_t m_au8IPv4[4];
		};
	};
	std::uint16_t m_u16Port;

public:
	PeerInfo(const void *pSockAddr, std::size_t uSockAddrLen);

	PeerInfo(bool bIPv6, std::uint16_t u16Port) noexcept; // IPv4_Any 或  IPv6_Any。
	PeerInfo(const std::uint16_t (&au16IPv6)[8], std::uint16_t u16Port) noexcept;
	PeerInfo(const std::uint8_t (&au8IPv4)[4], std::uint16_t u16Port) noexcept;

public:
	void ToIPv6(std::uint16_t (&au16IPv6)[8], std::uint16_t &u16Port) const noexcept;

	bool IsIPv4() const noexcept;
	void ToIPv4(std::uint8_t (&au8IPv4)[4], std::uint16_t &u16Port) const noexcept;
};

}

#endif
