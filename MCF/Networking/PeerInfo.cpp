// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "PeerInfo.hpp"
#include "../../MCFCRT/c/ext/assert.h"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "_SocketUtils.hpp"
using namespace MCF;

#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#	define	SWAP_IF_NOT_BIG_ENDIAN(x)	(x)
#else
#	define	SWAP_IF_NOT_BIG_ENDIAN(x)	(__builtin_bswap16(x))
#endif

// 构造函数和析构函数。
PeerInfo::PeerInfo(const void *pSockAddr, std::size_t uSockAddrLen){
	auto &vSockAddr = *(const SOCKADDR_STORAGE *)pSockAddr;
	if((vSockAddr.ss_family == AF_INET6) && (uSockAddrLen >= sizeof(SOCKADDR_IN6))){
		const auto &vSockAddrIn6 = reinterpret_cast<const SOCKADDR_IN6 &>(vSockAddr);
		BCopy(m_au16IPv6, vSockAddrIn6.sin6_addr);
		BCopy(m_u16Port, vSockAddrIn6.sin6_port);
	} else if((vSockAddr.ss_family == AF_INET) && (uSockAddrLen >= sizeof(SOCKADDR_IN))){
		const auto &vSockAddrIn = reinterpret_cast<const SOCKADDR_IN &>(vSockAddr);
		BZero(m_au16IPv4Zeroes);
		m_au16IPv4Ones = 0xFFFF;
		BCopy(m_au8IPv4, vSockAddrIn.sin_addr);
		BCopy(m_u16Port, vSockAddrIn.sin_port);
	} else {
		MCF_THROW(ERROR_NOT_SUPPORTED, L"不支持该协议。");
	}
}

PeerInfo::PeerInfo(bool bIPv6, std::uint16_t u16Port) noexcept {
	if(bIPv6){
		BZero(m_au16IPv6);
	} else {
		BZero(m_au16IPv4Zeroes);
		m_au16IPv4Ones = 0xFFFF;
		BZero(m_au8IPv4);
	}
	m_u16Port = SWAP_IF_NOT_BIG_ENDIAN(u16Port);
}
PeerInfo::PeerInfo(const std::uint16_t (&au16IPv6)[8], std::uint16_t u16Port) noexcept {
	for(std::size_t i = 0; i < 8; ++i){
		m_au16IPv6[i] = SWAP_IF_NOT_BIG_ENDIAN(au16IPv6[i]);
	}
	m_u16Port = SWAP_IF_NOT_BIG_ENDIAN(u16Port);
}
PeerInfo::PeerInfo(const std::uint8_t (&au8IPv4)[4], std::uint16_t u16Port) noexcept {
	BZero(m_au16IPv4Zeroes);
	m_au16IPv4Ones = 0xFFFF;
	BCopy(m_au8IPv4, au8IPv4);
	m_u16Port = SWAP_IF_NOT_BIG_ENDIAN(u16Port);
}

// 其他非静态成员函数。
bool PeerInfo::IsNull() const noexcept {
	switch(m_au16IPv4Ones){
	case 0:
		return !BTest(m_au16IPv6);
	case 0xFFFF:
		return !BTest(m_au8IPv4);
	default:
		return false;
	}
}
bool PeerInfo::IsIPv6Null() const noexcept {
	return !BTest(m_au16IPv6);
}
bool PeerInfo::IsIPv4Null() const noexcept {
	return IsIPv4() && !BTest(m_au8IPv4);
}

void PeerInfo::ToIPv6(std::uint16_t (&au16IPv6)[8], std::uint16_t &u16Port) const noexcept {
	for(std::size_t i = 0; i < 8; ++i){
		au16IPv6[i] = SWAP_IF_NOT_BIG_ENDIAN(m_au16IPv6[i]);
	}
	u16Port = SWAP_IF_NOT_BIG_ENDIAN(m_u16Port);
}

bool PeerInfo::IsIPv4() const noexcept {
	return !BTest(m_au16IPv4Zeroes) && (m_au16IPv4Ones == 0xFFFF);
}
void PeerInfo::ToIPv4(std::uint8_t (&au8IPv4)[4], std::uint16_t &u16Port) const noexcept {
	ASSERT(IsIPv4());

	BCopy(au8IPv4, m_au8IPv4);
	u16Port = SWAP_IF_NOT_BIG_ENDIAN(m_u16Port);
}
