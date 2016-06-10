#include <MCF/StdMCF.hpp>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
/*	MCF::StandardInputStream  in;
	MCF::StandardOutputStream out;
	MCF::StandardErrorStream  err;

	out.PutString(L"标准输出 1", 6, true);
	err.PutString(L"标准错误 1", 6, true);
	wchar_t str[10];
	auto len = in.GetString(str, 10, true);
	out.PutString(str, len, true);
*/
	_MCFCRT_Mutex m = { };
	_MCFCRT_WaitForMutexForever(&m, 0);
	_MCFCRT_ConditionVariable c = { };

	double t1, t2;
	t1 = _MCFCRT_GetHiResMonoClock();
	_MCFCRT_WaitForConditionVariable(&c, &__MCFCRT_GthreadUnlockCallbackMutex, &__MCFCRT_GthreadRelockCallbackMutex, (_MCFCRT_STD intptr_t)&m, _MCFCRT_GetFastMonoClock() + 1000);
	t2 = _MCFCRT_GetHiResMonoClock();
	__builtin_printf("dt = %f\n", t2 - t1);

	return 0;
}
