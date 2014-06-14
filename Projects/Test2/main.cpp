#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
//#include <MCF/Thread/ThreadLocalPtr.hpp>
#include <MCF/Thread/Thread.hpp>
#include <cstdio>
using namespace std;
using namespace MCF;

extern "C" int __mingwthr_key_dtor(unsigned long ulKey, void (*pfnDtor)(void *));
extern "C" int __mingwthr_remove_key_dtor(unsigned long ulKey);

unsigned int MCFMain(){
	try {
	//	ThreadLocalPtr<WideString> p;
	//	p->Assign(L"meow"_wso);
	//	printf("main -> %ls\n", p->GetCStr());

		Thread::Create([&]{
	//		printf("thread -> %ls\n", p->GetCStr());
			throw 12345;
		})->Join();
	} catch(int &e){
		printf("exception! %d\n", e);
	}
	return 0;
}
