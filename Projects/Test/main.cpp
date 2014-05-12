#include <MCF/StdMCF.hpp>
#include <MCF/Socket/TcpPeer.hpp>
#include <MCF/Core/Exception.hpp>
#include <cstring>
using namespace MCF;

unsigned int MCFMain(){
	try {
		auto pServer = TcpPeer::Connect(PeerInfo(127, 0, 0, 1, 802));
		for(;;){
			char buffer[100];
			if(std::scanf("%99s", buffer) < 1){
				break;
			}
			pServer->Write(buffer, std::strlen(buffer));
		}
	} catch(Exception &e){
		std::printf("Exception:\n");
		std::printf("  Func: %s\n", e.pszFunction);
		std::printf("  Code: %lu\n", e.ulErrorCode);
		std::printf("  Desc: %s\n", AnsiString(GetWin32ErrorDesc(e.ulErrorCode)).GetCStr());
	}

	return 0;
}
