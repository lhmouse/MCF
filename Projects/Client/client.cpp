#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Networking/TcpClient.hpp>

unsigned int MCFMain(){
	try {
		MCF::TcpClient clnt;
		clnt.Connect(MCF::PeerInfo::FromIPv4({127, 0, 0, 1}, 8001));
//		clnt.Connect(MCF::PeerInfo::FromIPv6({0, 0, 0, 0, 0, 0, 0, 1}, 8001));

		std::uint32_t data[2] = { 2, 3 };
		clnt.Write(&data, sizeof(data));

		std::uint32_t res;
		clnt.Read(&res, sizeof(res));

		std::printf("from server: %u", res);
	} catch(MCF::Exception &e){
		::__MCF_CRT_BailF(L"func = %s\nerr  = %lu\ndesc = %ls", e.m_pszFunction, e.m_ulCode, e.m_pwszMessage);
	}
	return 0;
}
