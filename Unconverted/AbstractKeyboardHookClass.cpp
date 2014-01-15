// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "AbstractKeyboardHookClass.hpp"
using namespace MCF;

// 静态成员变量。
ReadWriteLock								AbstractKeyboardHookClass::xs_Lock				__attribute__((init_priority(2000)));
AbstractKeyboardHookClass::xTHREADHOOKMAP	AbstractKeyboardHookClass::xs_mapThreadHooks	__attribute__((init_priority(2000)));

// 静态成员函数。
LRESULT CALLBACK AbstractKeyboardHookClass::xKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam){
	if(nCode >= 0){
		std::deque<AbstractKeyboardHookClass *> *pdeqThreadHookInstances = nullptr;

		xs_Lock.AcquireReadLock();
			const DWORD dwCurrentThreadID = ::GetCurrentThreadId();
			auto iterCurrentThreadHooks = xs_mapThreadHooks.find(dwCurrentThreadID);
			if(iterCurrentThreadHooks != xs_mapThreadHooks.end()){
				pdeqThreadHookInstances = &iterCurrentThreadHooks->second.deqpHookInstances;
			}
		xs_Lock.ReleaseReadLock();

		if(pdeqThreadHookInstances != nullptr){
			for(auto iter = pdeqThreadHookInstances->rbegin(); iter != pdeqThreadHookInstances->rend(); ++iter){
				if((*iter)->xOnKeyboardHook((int)(wParam & 0xFFFFFFFF), (WORD)(lParam & 0xFFFF), (BYTE)((lParam >> 16) & 0xFF), (lParam & (1 << 24)) != 0, (lParam & (1 << 29)) != 0, (lParam & (1 << 30)) != 0, (lParam & (1 << 31)) != 0)){
					return TRUE;
				}
			}
		}
	}
	return ::CallNextHookEx(NULL, nCode, wParam, lParam);
}

// 构造函数和析构函数。
AbstractKeyboardHookClass::AbstractKeyboardHookClass(){
	xs_Lock.AcquireWriteLock();
		const DWORD dwCurrentThreadID = ::GetCurrentThreadId();
		auto iterCurrentThreadHooks = xs_mapThreadHooks.find(dwCurrentThreadID);

		if(iterCurrentThreadHooks == xs_mapThreadHooks.end()){
			xTHREADHOOKS CurrentThreadHook;
			CurrentThreadHook.hHook = VERIFY(::SetWindowsHookEx(WH_KEYBOARD, &xKeyboardHookProc, NULL, dwCurrentThreadID));

			iterCurrentThreadHooks = xs_mapThreadHooks.insert(xs_mapThreadHooks.end(), std::make_pair(dwCurrentThreadID, CurrentThreadHook));
		}
		iterCurrentThreadHooks->second.deqpHookInstances.emplace_back(this);
	xs_Lock.ReleaseWriteLock();
}
AbstractKeyboardHookClass::~AbstractKeyboardHookClass(){
	xs_Lock.AcquireWriteLock();
		const DWORD dwCurrentThreadID = ::GetCurrentThreadId();
		auto iterCurrentThreadHooks = xs_mapThreadHooks.find(dwCurrentThreadID);
		auto &deqThreadHookInstances = iterCurrentThreadHooks->second.deqpHookInstances;

		for(auto iter = deqThreadHookInstances.begin(); iter != deqThreadHookInstances.end(); ++iter){
			if(*iter == this){
				deqThreadHookInstances.erase(iter);

				if(deqThreadHookInstances.size() == 0){
					::UnhookWindowsHookEx(iterCurrentThreadHooks->second.hHook);
					xs_mapThreadHooks.erase(iterCurrentThreadHooks);
				}
				break;
			}
		}
	xs_Lock.ReleaseWriteLock();
}
