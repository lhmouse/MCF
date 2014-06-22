#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Thread/ThreadLocalPtr.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/Event.hpp>
#include <cstdio>
using namespace std;
using namespace MCF;

template class ThreadLocalPtr<WideString>;

unsigned int MCFMain(){
	void *v[20];
	for(auto &u : v){
		u = MCF_CRT_TlsAllocKey(0);
		printf("allocated %p\n", u);
	}
	random_shuffle(begin(v), end(v));
	for(auto &u : v){
		MCF_CRT_TlsFreeKey(u);
		printf("freed %p\n", u);
	}

	const auto evn = Event::Create(false);
	ThreadLocalPtr<WideString> p;
	try {
		p->Assign(L"meow!-----------------------"_wso);
		auto pthrd = Thread::Create([&]{
			p->Assign(L"bark!-----------------------"_wso);
			printf("thread -> %ls\n", p->GetCStr());
			evn->Set();
			throw 12345;
		}, true);
		pthrd->Resume();
		evn->Wait();
		printf("main -> %ls\n", p->GetCStr());
		pthrd->Join();
	} catch(int &e){
		printf("exception! %d\n", e);
	}

	shared_ptr<Thread> threads[100];
	ThreadLocalPtr<WideString> tls[200];
	for(auto &p : threads){
		p = Thread::Create([&]{
			::Sleep(1000);
			for(auto &pstr : tls){
				pstr->Assign(L"hello world!-----------------------"_wso);
			}
		});
	}
	for(auto &p : threads){
		p->Join();
	}

	return 0;
}
