#include <MCF/StdMCF.hpp>
#include <MCF/Socket/TcpServer.hpp>
#include <MCF/Core/Exception.hpp>
using namespace MCF;

extern const char begin	__asm__("thunk_begin");
extern const char end	__asm__("thunk_end");

__asm__(
	"thunk_begin: \n"
	"lea eax, dword ptr[ecx + edx] \n"
	"ret \n"
	"thunk_end: \n"
);

unsigned int MCFMain(){
	try {
		auto pServer = TcpServer::Create(PeerInfo(127, 0, 0, 1, 802));
		for(;;){
			std::printf("Waiting for client...\n");
			auto pClient = pServer->GetPeer();
			if(!pClient){
				std::printf("  Bad client\n");
				continue;
			}

			std::uint8_t au8ClientAddr[4];
			std::uint16_t u16ClientPort;
			pClient->GetPeerInfo().ToIPv4(au8ClientAddr, u16ClientPort);
			std::printf(
				"  Client %u.%u.%u.%u:%u\n",
				au8ClientAddr[0],
				au8ClientAddr[1],
				au8ClientAddr[2],
				au8ClientAddr[3],
				u16ClientPort
			);

			try {
				for(;;){
					char achBuffer[256];
					auto uBytesRead = pClient->Read(achBuffer, sizeof(achBuffer) - 1);
					std::printf("    Read %u bytes\n", uBytesRead);
					if(uBytesRead == 0){
						std::printf("    Connection closed\n");
						break;
					}
					achBuffer[uBytesRead] = 0;

					std::printf("    String \"%s\"\n", achBuffer);
				}
			} catch(Exception &e){
				std::printf("    Exception:\n");
				std::printf("      Func: %s\n", e.pszFunction);
				std::printf("      Code: %lu\n", e.ulErrorCode);
				std::printf("      Desc: %s\n", AnsiString(GetWin32ErrorDesc(e.ulErrorCode)).GetCStr());
			}
		}
	} catch(...){
		std::printf("Exception in main.\n");
	}
	return 0;
}
