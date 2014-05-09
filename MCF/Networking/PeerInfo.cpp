// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "PeerInfo.hpp"
#include "../../MCFCRT/ext/assert.h"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "_SocketUtils.hpp"
using namespace MCF;

namespace {

template<typename T, std::size_t N>
inline bool IsAllZeroes(const T (&a)[N]) noexcept {
	return std::all_of(
		std::begin(a),
		std::end(a),
		[](auto v) noexcept { return v == 0; }
	);
}

}

namespace MCF {

PeerInfo xPeerInfoFromSockAddr(const void *pSockAddr, std::size_t uSockAddrLen){
	auto &vSockAddr = *(const SOCKADDR_STORAGE *)pSockAddr;
	if((vSockAddr.ss_family == AF_INET6) && (uSockAddrLen >= sizeof(SOCKADDR_IN6))){
		const auto &vSockAddrIn6 = reinterpret_cast<const SOCKADDR_IN6 &>(vSockAddr);
		PeerInfo vRet(true, 0);
		BCopy(vRet.m_au16IPv6, vSockAddrIn6.sin6_addr);
		BCopy(vRet.m_u16Port, vSockAddrIn6.sin6_port);
		return std::move(vRet);
	} else if((vSockAddr.ss_family == AF_INET) && (uSockAddrLen >= sizeof(SOCKADDR_IN))){
		const auto &vSockAddrIn = reinterpret_cast<const SOCKADDR_IN &>(vSockAddr);
		PeerInfo vRet(false, 0);
		BCopy(vRet.m_au8IPv4, vSockAddrIn.sin_addr);
		BCopy(vRet.m_u16Port, vSockAddrIn.sin_port);
		return std::move(vRet);
	} else {
		MCF_THROW(ERROR_NOT_SUPPORTED, L"不支持该协议。");
	}
}

}

// 构造函数和析构函数。
PeerInfo::PeerInfo(bool bIPv6, std::uint16_t u16Port) noexcept {
	if(bIPv6){
		BZero(m_au16IPv6);
	} else {
		BZero(m_au16IPv4Zeroes);
		m_au16IPv4Ones = 0xFFFF;
		BZero(m_au8IPv4);
	}
	m_u16Port = ::ntohs(u16Port);
}
PeerInfo::PeerInfo(
	std::uint16_t u16IPv6_0,
	std::uint16_t u16IPv6_1,
	std::uint16_t u16IPv6_2,
	std::uint16_t u16IPv6_3,
	std::uint16_t u16IPv6_4,
	std::uint16_t u16IPv6_5,
	std::uint16_t u16IPv6_6,
	std::uint16_t u16IPv6_7,
	std::uint16_t u16Port
) noexcept {
	m_au16IPv6[0]	= ::ntohs(u16IPv6_0);
	m_au16IPv6[1]	= ::ntohs(u16IPv6_1);
	m_au16IPv6[2]	= ::ntohs(u16IPv6_2);
	m_au16IPv6[3]	= ::ntohs(u16IPv6_3);
	m_au16IPv6[4]	= ::ntohs(u16IPv6_4);
	m_au16IPv6[5]	= ::ntohs(u16IPv6_5);
	m_au16IPv6[6]	= ::ntohs(u16IPv6_6);
	m_au16IPv6[7]	= ::ntohs(u16IPv6_7);
	m_u16Port		= ::ntohs(u16Port);
}
PeerInfo::PeerInfo(
	std::uint8_t u8IPv4_0,
	std::uint8_t u8IPv4_1,
	std::uint8_t u8IPv4_2,
	std::uint8_t u8IPv4_3,
	std::uint16_t u16Port
) noexcept {
	BZero(m_au16IPv4Zeroes);
	m_au16IPv4Ones = 0xFFFF;

	m_au8IPv4[0]	= u8IPv4_0;
	m_au8IPv4[1]	= u8IPv4_1;
	m_au8IPv4[2]	= u8IPv4_2;
	m_au8IPv4[3]	= u8IPv4_3;
	m_u16Port		= ::ntohs(u16Port);
}

// 其他非静态成员函数。
bool PeerInfo::IsNull() const noexcept {
	if(!IsAllZeroes(m_au16IPv4Zeroes)){
		return false;
	}
	if((m_au16IPv4Ones != 0) && (m_au16IPv4Ones != 0xFFFF)){
		return false;
	}
	if(!IsAllZeroes(m_au8IPv4)){
		return false;
	}
	return true;
}
bool PeerInfo::IsIPv6Null() const noexcept {
	return IsAllZeroes(m_au16IPv6);
}
bool PeerInfo::IsIPv4Null() const noexcept {
	return IsIPv4() && IsAllZeroes(m_au8IPv4);
}

void PeerInfo::ToIPv6(std::uint16_t (&au16IPv6)[8], std::uint16_t &u16Port) const noexcept {
	for(std::size_t i = 0; i < 8; ++i){
		au16IPv6[i] = ::ntohs(m_au16IPv6[i]);
	}
	u16Port = ::ntohs(m_u16Port);
}

bool PeerInfo::IsIPv4() const noexcept {
	return IsAllZeroes(m_au16IPv4Zeroes) && (m_au16IPv4Ones == 0xFFFF);
}
void PeerInfo::ToIPv4(std::uint8_t (&au8IPv4)[4], std::uint16_t &u16Port) const noexcept {
	ASSERT(IsIPv4());

	BCopy(au8IPv4, m_au8IPv4);
	u16Port = ::ntohs(m_u16Port);
}
