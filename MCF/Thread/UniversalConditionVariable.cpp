// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "UniversalConditionVariable.hpp"
using namespace MCF;

// 构造函数和析构函数。
UniversalConditionVariable::UniversalConditionVariable(std::size_t uSpinCount)
	: xm_vMutex(uSpinCount), xm_vDelegate(xm_vMutex)
{
}

// 其他非静态成员函数。
bool UniversalConditionVariable::Wait(UniqueLockTemplateBase &vLock, unsigned long long ullMilliSeconds) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	xm_vMutex.Lock();
	vLock.Unlock();

	const bool bResult = xm_vDelegate.Wait(ullMilliSeconds);

	vLock.Lock();
	xm_vMutex.Unlock();
	return bResult;
}
void UniversalConditionVariable::Wait(UniqueLockTemplateBase &vLock) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	xm_vMutex.Lock();
	vLock.Unlock();

	xm_vDelegate.Wait();

	vLock.Lock();
	xm_vMutex.Unlock();
}
void UniversalConditionVariable::Signal(std::size_t uMaxCount) noexcept {
	xm_vDelegate.Signal(uMaxCount);
}
void UniversalConditionVariable::Broadcast() noexcept {
	xm_vDelegate.Broadcast();
}
