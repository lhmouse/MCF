#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Networking/HttpClient.hpp>
#include "../../MCFCRT/cpp/ext/multi_indexed_map.hpp"
using namespace MCF;

unsigned int MCFMain(){
	try {
		HttpClient http;
		http.Connect(L"POST", L"https://localhost", "post_test1=meow1&post_test2=meow2", 33);
		http.Connect(L"GET", L"https://localhost/?get_test1=meow1&get_test2=meow2");
	} catch(Exception &e){
		Bail(L"MCF::Exception:\n\nFunc = %s\nCode = %lu\nDesc = %ls\nMsg  = %ls", e.pszFunction, e.ulErrorCode, GetWin32ErrorDesc(e.ulErrorCode).GetCStr(), e.pwszMessage);
	}

	MultiIndexedMap<int, Index<int, std::greater<int>>> mim;
	mim.Insert(1, 1);
	mim.Insert(2, 4);
	mim.Insert(3, 9);
	mim.Insert(4, 16);

	auto p = mim.GetBack<0>();
	while(p){
		std::printf("%d\n", p->GetIndex<0>());
		p = mim.GetPrev<0>(p);
	}

	return 0;
}
