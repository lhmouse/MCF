#include "StdMCF.hpp"
#include <cstdio>
#include <cstring>
#include "Core/SharedHandleNTS.hpp"
//#include "Core/Format.hpp"

struct S {
	S(){
		std::puts("ctor of S");
	}
	~S(){
		std::puts("dtor of S");
	}
};

typedef S *PS;

struct Deleter {
	PS operator()(){
		return nullptr;
	}
	void operator()(PS p){
		delete p;
	}
};

unsigned int MCFMain(){
	const PS *pp;
	{
		MCF::SharedHandleNTS<PS, Deleter> p1;
		{
			p1.Reset(new S);
			auto p2 = p1;
			std::puts("about to add ref 1");
			pp = p2.AddRef();
			std::puts("about to leave inner scope");
		}
		std::puts("about to leave outer scope");
	}
	std::puts("about to add ref 2");
	MCF::SharedHandleNTS<PS, Deleter>::AddRef(pp);
	std::puts("about to drop ref 1");
	MCF::SharedHandleNTS<PS, Deleter>::DropRef(pp);
	std::puts("about to drop ref 2");
	MCF::SharedHandleNTS<PS, Deleter>::DropRef(pp);
	std::puts("about to return from main");
	return 0;
}
