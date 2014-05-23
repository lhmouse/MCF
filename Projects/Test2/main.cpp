#include <MCF/StdMCF.hpp>
#include <MCF/Socket/UdpServer.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/Time.hpp>
#include <iostream>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	auto p = UdpServer::Create(PeerInfo(192, 168, 1, 2, 10880));
	p->JoinGroup(PeerInfo(239, 253, 101, 82, 10880));
	for(;;){
		auto pkt = p->GetPacketTimeout(WAIT_FOREVER);
		if(!pkt){
			continue;
		}
		auto &buf = pkt->m_sbufData;
		copy(buf.GetReadIterator(), buf.GetReadEnd(), ostream_iterator<char>(cout));
		cout <<endl;
	}

	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->pszFunction);
		printf("  line = %lu\n", p->ulLine);
		printf("  msg  = %s\n", AnsiString(WideString(p->pwszMessage)).GetCStr());
	}
	return 0;
}
