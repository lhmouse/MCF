// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_ABSTRACT_MOUSE_HOOK_CLASS_HPP__
#define __MCF_ABSTRACT_MOUSE_HOOK_CLASS_HPP__

#include "StdMCF.hpp"
#include <map>
#include <deque>

namespace MCF {
	class AbstractMouseHookClass : NO_COPY_OR_ASSIGN {
	private:
		typedef struct xtagThreadHooks {
			HHOOK hHook;
			std::deque<AbstractMouseHookClass *> deqpHookInstances;
		} xTHREADHOOKS;
		typedef std::map<DWORD, xTHREADHOOKS> xTHREADHOOKMAP;

		static ReadWriteLock xs_Lock;
		static xTHREADHOOKMAP xs_mapThreadHooks;
	private:
		static LRESULT CALLBACK xMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	public:
		AbstractMouseHookClass();
		virtual ~AbstractMouseHookClass();
	protected:
		virtual bool xOnMouseHook(UINT_PTR uEventID, POINT pntCursorPos, HWND hWnd, UINT uHitTestCode, ULONG_PTR ulExtraInfo) = 0;
	};
}

#endif
