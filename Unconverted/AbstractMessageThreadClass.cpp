// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "AbstractMessageThreadClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
AbstractMessageThreadClass::AbstractMessageThreadClass() :
	xm_ReadyEvent(false)
{
}
AbstractMessageThreadClass::~AbstractMessageThreadClass(){
	QuitMessageLoop(0);
	Wait();
}

// 其他非静态成员函数。
void AbstractMessageThreadClass::xPostCreate(){
}
void AbstractMessageThreadClass::xPreTerminate(int nExitCode){
	UNREF_PARAM(nExitCode);
}

int AbstractMessageThreadClass::xThreadProc(){
	MSG msgDummy;
	::PeekMessage(&msgDummy, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	xPostCreate();
	xm_ReadyEvent.Set();
	const int nExitCode = xDoMessageLoop();
	xPreTerminate(nExitCode);

	return nExitCode;
}
int AbstractMessageThreadClass::xDoMessageLoop(){
	::MSG Msg;

	for(;;){
		while(::PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE) == FALSE){
			// 若没有消息则执行空闲循环。
			if(!xDoOnIdle()){
				// 空闲循环返回 false，则调用 GetMessage() 进入睡眠状态。
				if(::GetMessage(&Msg, NULL, 0, 0) == FALSE){
					return (int)Msg.wParam;
				}
				break;
			}
		}
		// 此时 Msg 有效，并且是从消息队列中移除的。
		do {
			if(Msg.message == WM_QUIT){
				return (int)Msg.wParam;
			}
			xDoOnMessage(&Msg);
		} while(::PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE) != FALSE);
	}
}

void AbstractMessageThreadClass::Create(bool bSuspended){
	xm_ReadyEvent.Clear();
	AbstractThreadClass::Create(false);
	xm_ReadyEvent.Wait();

	if(bSuspended){
		Suspend();
	}
}

void AbstractMessageThreadClass::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam){
	const DWORD dwThreadID = GetThreadID();
	if(dwThreadID != 0){
		::PostThreadMessage(dwThreadID, uMsg, wParam, lParam);
	}
}
void AbstractMessageThreadClass::QuitMessageLoop(int nExitCode){
	PostMessage(WM_QUIT, (WPARAM)nExitCode, 0);
}
