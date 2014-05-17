#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ThreadLocalPtr.hpp>
using namespace MCF;

ThreadLocalPtr<char> c;

unsigned int MCFMain(){
	*c = 'a';
	return 0;
}
