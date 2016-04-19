// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "static_ctors.h"
#include "mcfwin.h"
#include <exception>
#include <cstdint>
#include <csetjmp>

extern "C" {

extern void __main();

extern std::jmp_buf *__MCFCRT_pjbufAbortHook;

bool __MCFCRT_CallStaticCtors() noexcept {
	// 如果在 DLL 的静态构造函数抛出异常，我们不终止进程，而是返回 false。
	// 这将导致 DllMain() 返回 FALSE，而不会终止当前进程。

	int nResult;
	std::jmp_buf jbufHook;

	const auto pjbufOldHook = ::__MCFCRT_pjbufAbortHook;
	::__MCFCRT_pjbufAbortHook = &jbufHook;
	{
		if((nResult = setjmp(jbufHook)) == 0){
			try {
				::__main();
			} catch(...){
				std::terminate();
			}
		}
	}
	::__MCFCRT_pjbufAbortHook = pjbufOldHook;

	if(nResult != 0){
		::SetLastError((DWORD)nResult);
		return false;
	}
	return true;
}
void __MCFCRT_CallStaticDtors() noexcept {
}

}
