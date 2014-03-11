#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Networking/UdpServer.hpp>
#include <MCF/Networking/PeerInfo.hpp>

static void print_pi(const MCF::PeerInfo &pi) noexcept {
	const auto is_ipv4 = pi.IsIPv4();
	std::printf("ipv%c, ", is_ipv4 ? '4' : '6');
	if(is_ipv4){
		std::uint8_t au8IPv4[4];
		std::uint16_t u16Port;
		pi.ToIPv4(au8IPv4, u16Port);
		std::printf("ip:port = %d.%d.%d.%d:%u\n", au8IPv4[0], au8IPv4[1], au8IPv4[2], au8IPv4[3], u16Port);
	} else {
		std::uint16_t au16IPv6[8];
		std::uint16_t u16Port;
		pi.ToIPv6(au16IPv6, u16Port);
		std::printf(
			"ip:port = %04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X:%u\n",
			au16IPv6[0], au16IPv6[1], au16IPv6[2], au16IPv6[3],
			au16IPv6[4], au16IPv6[5], au16IPv6[6], au16IPv6[7],
			u16Port
		);
	}
}

static void foo(MCF::UdpPacket packet) noexcept {
	std::printf("--- peer info - ");
	print_pi(packet.GetPeerInfo());

	packet.GetBuffer().Push(0);
	std::printf("--- data - %s\n", packet.GetBuffer().GetData());
}

unsigned int MCFMain(){
	try {
		MCF::UdpServer srvr;
		srvr.Start(MCF::PeerInfo(true, 8001));
		for(;;){
			foo(srvr.GetPacket());
		}
	} catch(MCF::Exception &e){
		::__MCF_CRT_BailF(L"func = %s\nerr  = %lu\ndesc = %ls", e.m_pszFunction, e.m_ulCode, e.m_pwszMessage);
	}
	return 0;
}
