#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Networking/TcpServer.hpp>

static void foo(std::unique_ptr<MCF::TcpPeer> client) noexcept {
	std::uint32_t src[2];
	client->Read(&src, sizeof(src));

	std::printf("from client: %u, %u\n", src[0], src[1]);

	std::uint32_t res = src[0] + src[1];
	client->Write(&res, sizeof(res));
}

unsigned int MCFMain(){
	try {
		MCF::TcpServer srvr(foo);
		srvr.Start(MCF::PeerInfo::FromIPv4({127, 0, 0, 1}, 8001), 8, true);
//		srvr.Start(MCF::PeerInfo::FromIPv6({0, 0, 0, 0, 0, 0, 0, 1}, 8001), 8, true);
		while(srvr.IsRunning()){
			::Sleep(1000);
		}
	} catch(MCF::Exception &e){
		::__MCF_CRT_BailF(L"func = %s\nerr  = %lu\ndesc = %ls", e.m_pszFunction, e.m_ulCode, e.m_pwszMessage);
	}
	return 0;
}
