#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/Thread.hpp>
using namespace MCF;

void thread(){
	throw 123;
}

unsigned int MCFMain() try {
	Thread::Create(thread)->Join();
	return 0;
} catch(std::exception &e){
	__builtin_printf("exception caught: %s\n", e.what());

	auto *const e2 = dynamic_cast<Exception *>(&e);
	if(e2){
		AnsiString desc(GetWin32ErrorDesc(e2->ulErrorCode));
		if((desc.GetSize() >= 2) && (desc.GetEnd()[-2] == '\r') && (desc.GetEnd()[-1] == '\n')){
			desc.Truncate(2);
		}

		__builtin_printf("  function = %s:%lu\n", e2->pszFunction, e2->ulLine);
		__builtin_printf("  err code = %lu\n", e2->ulErrorCode);
		__builtin_printf("  err desc = %s\n", desc.GetCStr());
		__builtin_printf("  message  = %s\n", AnsiString(WideString(e2->pwszMessage)).GetCStr());
	}
	return 1;
}

