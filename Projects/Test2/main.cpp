#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Socket/UdpServer.hpp>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
/*
	auto svr = UdpServer::Create(PeerInfo(127,0,0,1, 10880));
	for(;;){
		auto pkt = svr->GetPacketTimeout(1000);
		if(!pkt.pSender){
			puts("listening...");
			continue;
		}

		unsigned char ip[4];
		unsigned short port;
		pkt.pSender->GetPeerInfo().ToIPv4(ip, port);
		printf("remote = %hhu.%hhu.%hhu.%hhu:%hu\n", ip[0], ip[1], ip[2], ip[3], port);
		printf("packet size = %zu\n  contents: ", pkt.vecPayload.GetSize());
		fwrite(pkt.vecPayload.GetData(), pkt.vecPayload.GetSize(), 1, stdout);
		putchar('\n');
	}
*/
	auto snd = UdpSender::Create(PeerInfo(127,0,0,1, 10881), PeerInfo(127,0,0,1, 20100));
	for(;;){
		::Sleep(1000);
		char buf[20];
		int cnt = snprintf(buf, sizeof(buf), "%d", rand());
		snd->Send(buf, (unsigned)cnt);
		printf("sent %s\n", buf);
	}

	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->m_ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->m_pszFunction);
		printf("  line = %lu\n", p->m_ulLine);
		printf("  msg  = %s\n", AnsiString(p->m_wcsMessage).GetCStr());
	}
	return 0;
}
