#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ThreadLocalPtr.hpp>
#include <string>
using namespace MCF;

ThreadLocalPtr<std::string> tls;

extern "C" unsigned int MCFMain() noexcept {
	for(int i = 0; i < 1000; ++i){
		tls->append("a ");
	}
	return 0;
}
