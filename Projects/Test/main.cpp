#include <MCF/StdMCF.hpp>
#include <MCF/Socket/TcpPeer.hpp>
#include <MCF/Thread/Thread.hpp>
#include <array>
using namespace MCF;

unsigned int MCFMain(){
	std::array<std::shared_ptr<Thread>, 4> threads;
	for(auto &p : threads){
		p = Thread::Create([&]{
			for(;;){
				try {
					TcpPeer::Connect(PeerInfo(127, 0, 0, 1, 802));
				} catch(...){
				}
				::Sleep(1);
			}
		});
	}
	for(auto &p : threads){
		p->Join();
	}
	return 0;
}
