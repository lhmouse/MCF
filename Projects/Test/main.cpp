#include <MCF/StdMCF.hpp>
#include <MCF/Socket/UdpServer.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/Time.hpp>
#include <iostream>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	StreamBuffer buf;
	int i = 0;
	for(;;){
		char data[100];
		auto len = (unsigned int)sprintf(data, "i = %d", i++);
		buf.Clear();
		cout <<"will send '" <<data <<"'" <<endl;
		UdpPacket(PeerInfo(239, 253, 101, 82, 10880), data, len).Send();
		::Sleep(1000);
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
