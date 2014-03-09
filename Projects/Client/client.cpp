#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Networking/TcpPeer.hpp>
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

unsigned int MCFMain(){
	try {
		const unsigned char ipv4[] = {127, 0, 0, 1};
		const unsigned short ipv6[] = {0, 0, 0, 0, 0, 0, 0, 1};

		MCF::TcpPeer client;
		client.Connect(MCF::PeerInfo(ipv6, 8001));

		const std::uint32_t data[2] = { 2, 3 };
		for(;;){
			std::puts("--- peer info --");
			print_pi(client.GetPeerInfo());

			client.Write(&data, sizeof(data));

			std::uint32_t res;
			client.Read(&res, sizeof(res));

			std::printf("from server: %u\n", res);

			::Sleep(1000);
		}
	} catch(MCF::Exception &e){
		::__MCF_CRT_BailF(L"func = %s\nerr  = %lu\ndesc = %ls", e.m_pszFunction, e.m_ulCode, e.m_pwszMessage);
	}
	return 0;
}
