#include <MCF/StdMCF.hpp>
#include <MCF/Socket/TcpServer.hpp>
#include <MCF/Thread/Thread.hpp>
#include <array>
using namespace MCF;

unsigned int MCFMain(){
	std::array<std::shared_ptr<Thread>, 4> threads;
	auto pServer = TcpServer::Create(PeerInfo(127, 0, 0, 1, 802));
	for(auto &p : threads){
		p = Thread::Create([&]{
			for(;;){
				try {
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
				} catch(...){
				}
			}
		});
	}
	for(auto &p : threads){
		p->Join();
	}
	return 0;
}
