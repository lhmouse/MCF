// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_PEER_INFO_HPP_
#define MCF_PEER_INFO_HPP_

#include "../Core/Utilities.hpp"
#include <cstdint>

namespace MCF {

class PeerInfo {
private:
	union {
		std::uint16_t xm_au16IPv6[8];
		struct {
			std::uint16_t xm_au16IPv4Zeroes[5];
			std::uint16_t xm_au16IPv4Ones;
			std::uint8_t xm_au8IPv4[4];
		};
	};
	std::uint16_t xm_u16Port;

public:
	PeerInfo(
		bool bIPv6,
		std::uint16_t u16Port
	) noexcept; // 空地址.

	PeerInfo(
		const void *pSockAddr,
		std::size_t uSockAddrSize
	);

	PeerInfo(
		std::uint16_t u16IPv6_0,
		std::uint16_t u16IPv6_1,
		std::uint16_t u16IPv6_2,
		std::uint16_t u16IPv6_3,
		std::uint16_t u16IPv6_4,
		std::uint16_t u16IPv6_5,
		std::uint16_t u16IPv6_6,
		std::uint16_t u16IPv6_7,
		std::uint16_t u16Port
	) noexcept;

	PeerInfo(
		std::uint8_t u8IPv4_0,
		std::uint8_t u8IPv4_1,
		std::uint8_t u8IPv4_2,
		std::uint8_t u8IPv4_3,
		std::uint16_t u16Port
	) noexcept;

public:
	bool IsNull() const noexcept;
	bool IsIPv6Null() const noexcept;
	bool IsIPv4Null() const noexcept;

	int ToSockAddr(void *pSockAddr, std::size_t uSockAddrSize) const noexcept;

	void ToIPv6(std::uint16_t (&au16IPv6)[8], std::uint16_t &u16Port) const noexcept;

	bool IsIPv4() const noexcept;
	void ToIPv4(std::uint8_t (&au8IPv4)[4], std::uint16_t &u16Port) const noexcept;

public:
	explicit operator bool() const noexcept {
		return !IsNull();
	}

	bool operator==(const PeerInfo &rhs) const noexcept {
		const int nResult = BComp(xm_au16IPv6, rhs.xm_au16IPv6);
		if(nResult != 0){
			return false;
		}
		return xm_u16Port == rhs.xm_u16Port;
	}
	bool operator!=(const PeerInfo &rhs) const noexcept {
		return !(*this == rhs);
	}
	bool operator<(const PeerInfo &rhs) const noexcept {
		const int nResult = BComp(xm_au16IPv6, rhs.xm_au16IPv6);
		if(nResult != 0){
			return nResult < 0;
		}
		return xm_u16Port < rhs.xm_u16Port;
	}
	bool operator>(const PeerInfo &rhs) const noexcept {
		return rhs < *this;
	}
	bool operator<=(const PeerInfo &rhs) const noexcept {
		return !(rhs < *this);
	}
	bool operator>=(const PeerInfo &rhs) const noexcept {
		return !(*this < rhs);
	}
};

}

#endif
