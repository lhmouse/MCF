// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "static_ctors.h"
#include "mcfwin.h"
#include <exception>
#include <cstdint>
#include <csetjmp>

extern "C" {

using CallbackProc = void (*)();

extern const CallbackProc __CTOR_LIST__[];
extern const CallbackProc __DTOR_LIST__[];

bool __MCF_CRT_CallStaticCtors() noexcept {
	// 如果静态构造函数抛出异常，我们不终止进程，而是返回 false。
	// 对于 EXE 这将调用 abort()，对于 DLL 这将导致 DllMain() 返回 FALSE。
	static std::jmp_buf s_vJmpBuf;

	const auto pfnOldTerminate = std::set_terminate([]{ std::longjmp(s_vJmpBuf, ERROR_PROCESS_ABORTED); });

	int nResult;
	if((nResult = setjmp(s_vJmpBuf)) != 0){
		std::set_terminate(pfnOldTerminate);
		::SetLastError((DWORD)nResult);
		return false;
	}

	auto ppfnCallback = __CTOR_LIST__ + 1;
	const auto uCount = reinterpret_cast<const std::uintptr_t &>(__CTOR_LIST__[0]);
	if(uCount == (std::uintptr_t)-1){
		while(*ppfnCallback){
			(*ppfnCallback)();
			++ppfnCallback;
		}
	} else {
		for(std::size_t i = 0; i < uCount; ++i){
			(*ppfnCallback)();
			++ppfnCallback;
		}
	}
	std::set_terminate(pfnOldTerminate);
	return true;
}
void __MCF_CRT_CallStaticDtors() noexcept {
	auto ppfnCallback = __DTOR_LIST__ + 1;
	const auto uCount = reinterpret_cast<const std::uintptr_t &>(__DTOR_LIST__[0]);
	if(uCount == (std::uintptr_t)-1){
		while(*ppfnCallback){
			(*ppfnCallback)();
			++ppfnCallback;
		}
	} else {
		for(std::size_t i = 0; i < uCount; ++i){
			(*ppfnCallback)();
			++ppfnCallback;
		}
	}
}

}
