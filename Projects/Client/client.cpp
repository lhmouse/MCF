#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Networking/UdpPacket.hpp>
#include <MCF/Networking/PeerInfo.hpp>

unsigned int MCFMain(){
	try {
	//	const unsigned char ipv4[] = {127, 0, 0, 1};
		const unsigned short ipv6[] = {0, 0, 0, 0, 0, 0, 0, 1};

		MCF::UdpPacket client(MCF::PeerInfo(ipv6, 8001));

		for(;;){
			client.GetBuffer().Push('a');
			client.Send();

			::Sleep(10);
		}
	} catch(MCF::Exception &e){
		::__MCF_CRT_BailF(L"func = %s\nerr  = %lu\ndesc = %ls", e.m_pszFunction, e.m_ulCode, e.m_pwszMessage);
	}
	return 0;
}
