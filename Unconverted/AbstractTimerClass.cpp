// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "AbstractTimerClass.hpp"
using namespace MCF;

// 静态成员函数。
void CALLBACK AbstractTimerClass::xTimerProc(UINT /* uTimerID */, UINT, DWORD_PTR dwUser, DWORD_PTR, DWORD_PTR){
	AbstractTimerClass *const pTimer = (AbstractTimerClass *)dwUser;

	ASSERT(pTimer != nullptr);

	pTimer->xTimerLoop();
}

// 构造函数和析构函数。
AbstractTimerClass::AbstractTimerClass(unsigned int uInterval, unsigned int uPrecision) :
	xm_uInterval(uInterval),
	xm_uPrecision(uPrecision),
	xm_TimerID(0)
{
}
AbstractTimerClass::~AbstractTimerClass(){
	Stop();
}

// 其他非静态成员函数。
void AbstractTimerClass::Start(){
	Stop();
	xm_TimerID = VERIFY(::timeSetEvent(xm_uInterval, xm_uPrecision, &xTimerProc, (DWORD_PTR)this, TIME_PERIODIC | TIME_CALLBACK_FUNCTION | TIME_KILL_SYNCHRONOUS));
}
void AbstractTimerClass::Stop(){
	if(xm_TimerID != 0){
		::timeKillEvent(xm_TimerID);
		xm_TimerID = 0;
	}
}
bool AbstractTimerClass::IsActive() const {
	return xm_TimerID != 0;
}
