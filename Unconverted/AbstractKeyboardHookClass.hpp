// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_ABSTRACT_KEYBOARD_HOOK_CLASS_HPP__
#define __MCF_ABSTRACT_KEYBOARD_HOOK_CLASS_HPP__

#include "StdMCF.hpp"
#include <map>
#include <deque>

namespace MCF {
	class AbstractKeyboardHookClass : NO_COPY_OR_ASSIGN {
	private:
		typedef struct xtagThreadHooks {
			HHOOK hHook;
			std::deque<AbstractKeyboardHookClass *> deqpHookInstances;
		} xTHREADHOOKS;
		typedef std::map<DWORD, xTHREADHOOKS> xTHREADHOOKMAP;

		static ReadWriteLock xs_Lock;
		static xTHREADHOOKMAP xs_mapThreadHooks;
	private:
		static LRESULT CALLBACK xKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	public:
		AbstractKeyboardHookClass();
		virtual ~AbstractKeyboardHookClass();
	protected:
		virtual bool xOnKeyboardHook(int nVirtualKeyCode, WORD wRepeatCount, BYTE byScanCode, bool bIsExtended, bool bIsSysKey, bool bIsPreviouslyDown, bool bIsBeingReleased) = 0;
	};
}

#endif
