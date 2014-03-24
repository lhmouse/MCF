#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Networking/HttpClient.hpp>
using namespace MCF;

unsigned int MCFMain(){
	try {
		HttpClient http;
		http.Connect(L"POST", L"https://localhost", "post_test=meow", 14);
		http.Connect(L"GET", L"https://localhost/?get_test=meow");
	} catch(Exception &e){
		Bail(L"MCF::Exception:\n\nFunc = %s\nCode = %lu\nDesc = %ls\nMsg  = %ls", e.pszFunction, e.ulErrorCode, GetWin32ErrorDesc(e.ulErrorCode).GetCStr(), e.pwszMessage);
	}

	return 0;
}
