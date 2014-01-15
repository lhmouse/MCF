// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "Time.hpp"
#include "WindowBaseClass.hpp"
#include "xThreadParasiteClass.hpp"
#include "xStatusBarClass.hpp"
using namespace MCF;

// 静态成员函数定义。
LRESULT CALLBACK WindowBaseClass::xWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	WindowBaseClass *const pWindowInstance = GetWindowPtr(hWnd);

	ASSERT(pWindowInstance != nullptr);
	ASSERT(pWindowInstance->xm_hWnd == hWnd);

	const LRESULT lResult = pWindowInstance->xOnMessage(uMsg, wParam, lParam);
	if(uMsg == WM_NCDESTROY){
		pWindowInstance->xDestroySubclassChain();
	}
	return lResult;
}

DWORD WindowBaseClass::xMakeAcceleratorMapKey(bool bCtrlCombined, bool bAltCombined, bool bShiftCombined, char chVKCode){
	return (DWORD)((unsigned char)chVKCode | (bCtrlCombined ? 0x100 : 0) | (bAltCombined ? 0x200 : 0) | (bShiftCombined ? 0x400 : 0));
}

WindowBaseClass *WindowBaseClass::GetWindowPtr(HWND hWnd){
	if(hWnd == NULL){
		return nullptr;
	}

	ASSERT(::IsWindow(hWnd));

	return (WindowBaseClass *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

// 构造函数和析构函数。
WindowBaseClass::WindowBaseClass(){
	xm_pfnOldWndProc	= nullptr;
	xm_hWnd				= NULL;
	xm_pPrevSubclasser	= nullptr;
	xm_pNextSubclasser	= nullptr;
}
WindowBaseClass::~WindowBaseClass(){
	Unsubclass();
}

// 其他非静态成员函数。
LRESULT WindowBaseClass::xDefSubclassProc(UINT uMsg, WPARAM wParam, LPARAM lParam){
	if(xm_pNextSubclasser != nullptr){
		return xm_pNextSubclasser->xOnMessage(uMsg, wParam, lParam);
	}

	ASSERT(xm_pfnOldWndProc != nullptr);

	return ::CallWindowProc(xm_pfnOldWndProc, xm_hWnd, uMsg, wParam, lParam);
}
void WindowBaseClass::xDestroySubclassChain(){
	WindowBaseClass *pCurrent = this;

	WindowBaseClass *pPrev = pCurrent->xm_pPrevSubclasser;
	while(pPrev != nullptr){
		pCurrent = pPrev;
		pPrev = pCurrent->xm_pPrevSubclasser;
	}
	do {
		WindowBaseClass *const pNext = pCurrent->xm_pNextSubclasser;
		pCurrent->Unsubclass();
		pCurrent = pNext;
	} while(pCurrent != nullptr);
}

void WindowBaseClass::xDoSetOnCommand(int nControlID, DWORD dwNotifyCode, WindowBaseClass::xLPFNONCOMMANDPROC pfnOnCommandProc){
	xm_mapOnCommandProcs[nControlID].emplace(std::make_pair(dwNotifyCode, pfnOnCommandProc));
}
void WindowBaseClass::xDoSetOnNotify(int nControlID, WindowBaseClass::xLPFNONNOTIFYPROC pfnOnNotifyProc){
	xm_mapOnNotifyProcs.emplace(std::make_pair(nControlID, pfnOnNotifyProc));
}

void WindowBaseClass::xPostSubclass(){
}
void WindowBaseClass::xPreUnsubclass(){
}

LRESULT WindowBaseClass::xOnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam){
	xm_RawWParam = wParam;
	xm_RawLParam = lParam;

	switch(uMsg){
		case WM_DESTROY:
			xOnDestroy();
			return 0;
		case WM_CLOSE:
			xOnClose();
			return 0;
		case WM_SHOWWINDOW:
			xOnShow(
				wParam != FALSE,
				(int)lParam
			);
			return 0;
		case WM_ERASEBKGND:
			return xOnEraseBkgnd(
				(HDC)wParam
			) ? TRUE : FALSE;
		case WM_PAINT:
			xOnPaint();
			return 0;
		case WM_MOVE:
			xOnMove(
				xm_RawLParam.Extract<INT16>(0, 16),
				xm_RawLParam.Extract<INT16>(16, 16)
			);
			return 0;
		case WM_SIZE:
			xOnSize(
				(int)wParam,
				xm_RawLParam.Extract<UINT16>(0, 16),
				xm_RawLParam.Extract<UINT16>(16, 16)
			);
			return 0;
		case WM_COMMAND:
			return xOnCommand(
				xm_RawWParam.Extract<INT16>(0, 16),
				xm_RawWParam.Extract<WORD>(16, 16),
				(HWND)lParam
			) ? TRUE : FALSE;
		case WM_SYSCOMMAND:
			return xOnSysCommand(
				(int)wParam,
				xm_RawLParam.Extract<INT16>(0, 16),
				xm_RawLParam.Extract<INT16>(16, 16)
			) ? TRUE : FALSE;
		case WM_NOTIFY:
			return xOnNotify(
				((LPNMHDR)lParam)->code,
				wParam,
				((LPNMHDR)lParam)->hwndFrom,
				(LPNMHDR)lParam
			) ? TRUE : FALSE;
		case WM_TIMER:
			xOnTimer(
				wParam,
				(TIMERPROC)lParam
			);
			return 0;
		case WM_KEYDOWN:
			xOnKeyDown(
				(int)wParam,
				xm_RawLParam.Extract<WORD>(0, 16),
				xm_RawLParam.Extract<BYTE>(16, 8),
				xm_RawLParam.Extract<bool>(24, 1),
				xm_RawLParam.Extract<bool>(30, 1)
			);
			return 0;
		case WM_KEYUP:
			xOnKeyUp(
				(int)wParam,
				xm_RawLParam.Extract<BYTE>(16, 8),
				xm_RawLParam.Extract<bool>(24, 1)
			);
			return 0;
		case WM_SYSKEYDOWN:
			xOnSysKeyDown(
				(int)wParam,
				xm_RawLParam.Extract<WORD>(0, 16),
				xm_RawLParam.Extract<BYTE>(16, 8),
				xm_RawLParam.Extract<bool>(24, 1),
				xm_RawLParam.Extract<bool>(29, 1),
				xm_RawLParam.Extract<bool>(30, 1)
			);
			return 0;
		case WM_SYSKEYUP:
			xOnSysKeyUp(
				(int)wParam,
				xm_RawLParam.Extract<BYTE>(16, 8),
				xm_RawLParam.Extract<bool>(24, 1),
				xm_RawLParam.Extract<bool>(29, 1)
			);
			return 0;
		case WM_LBUTTONDOWN:
			xOnLButtonDown(
				xm_RawWParam.ExtractBool(MK_CONTROL),
				xm_RawWParam.ExtractBool(MK_SHIFT),
				xm_RawWParam.ExtractBool(MK_MBUTTON),
				xm_RawWParam.ExtractBool(MK_RBUTTON),
				xm_RawLParam.Extract<INT16>(0, 16),
				xm_RawLParam.Extract<INT16>(16, 16)
			);
			return 0;
		case WM_LBUTTONUP:
			xOnLButtonUp(
				xm_RawWParam.ExtractBool(MK_CONTROL),
				xm_RawWParam.ExtractBool(MK_SHIFT),
				xm_RawWParam.ExtractBool(MK_MBUTTON),
				xm_RawWParam.ExtractBool(MK_RBUTTON),
				xm_RawLParam.Extract<INT16>(0, 16),
				xm_RawLParam.Extract<INT16>(16, 16)
			);
			return 0;
		case WM_RBUTTONDOWN:
			xOnRButtonDown(
				xm_RawWParam.ExtractBool(MK_CONTROL),
				xm_RawWParam.ExtractBool(MK_SHIFT),
				xm_RawWParam.ExtractBool(MK_LBUTTON),
				xm_RawWParam.ExtractBool(MK_MBUTTON),
				xm_RawLParam.Extract<INT16>(0, 16),
				xm_RawLParam.Extract<INT16>(16, 16)
			);
			return 0;
		case WM_RBUTTONUP:
			xOnRButtonUp(
				xm_RawWParam.ExtractBool(MK_CONTROL),
				xm_RawWParam.ExtractBool(MK_SHIFT),
				xm_RawWParam.ExtractBool(MK_LBUTTON),
				xm_RawWParam.ExtractBool(MK_MBUTTON),
				xm_RawLParam.Extract<INT16>(0, 16),
				xm_RawLParam.Extract<INT16>(16, 16)
			);
			return 0;
		case WM_MBUTTONDOWN:
			xOnMButtonDown(
				xm_RawWParam.ExtractBool(MK_CONTROL),
				xm_RawWParam.ExtractBool(MK_SHIFT),
				xm_RawWParam.ExtractBool(MK_LBUTTON),
				xm_RawWParam.ExtractBool(MK_RBUTTON),
				xm_RawLParam.Extract<INT16>(0, 16),
				xm_RawLParam.Extract<INT16>(16, 16)
			);
			return 0;
		case WM_MBUTTONUP:
			xOnMButtonUp(
				xm_RawWParam.ExtractBool(MK_CONTROL),
				xm_RawWParam.ExtractBool(MK_SHIFT),
				xm_RawWParam.ExtractBool(MK_LBUTTON),
				xm_RawWParam.ExtractBool(MK_RBUTTON),
				xm_RawLParam.Extract<INT16>(0, 16),
				xm_RawLParam.Extract<INT16>(16, 16)
			);
			return 0;
		case WM_SETCURSOR:
			return xOnSetCursor(
				(HWND)wParam,
				xm_RawLParam.Extract<INT16>(0, 16),
				xm_RawLParam.Extract<UINT16>(16, 16)
			) ? TRUE : FALSE;
		default:
			return xDefSubclassProc(uMsg, wParam, lParam);
	}
}

void WindowBaseClass::xOnInitialize(){
};
void WindowBaseClass::xOnDestroy(){
	xDefSubclassProc(WM_DESTROY, 0, 0);
};
void WindowBaseClass::xOnClose(){
	xDefSubclassProc(WM_CLOSE, 0, 0);
};
void WindowBaseClass::xOnShow(bool bToShow, int nStatus){
	xDefSubclassProc(WM_SHOWWINDOW, (WPARAM)(bToShow ? TRUE : FALSE), (LPARAM)nStatus);
};
bool WindowBaseClass::xOnEraseBkgnd(HDC hDC){
	return xDefSubclassProc(WM_ERASEBKGND, (WPARAM)hDC, 0) != 0;
}
void WindowBaseClass::xOnPaint(){
	xDefSubclassProc(WM_PAINT, 0, 0);
};
void WindowBaseClass::xOnMove(int nNewPosX, int nNewPosY){
	// 窗体位置的 X 坐标和 Y 坐标都是 32 位的，但是两个参数事实上是从 wParam 中取高低 16 位扩展而来的。
	// 所以如果窗体位置位于 -32768 ~ +32767 之外会出现问题，虽然这种情况通常不会发生。
	// 如果要获得窗体位置尽量使用 GetWindowRect()。
	xm_RawLParam.Combine<INT16>(0, 16, (INT16)nNewPosX);
	xm_RawLParam.Combine<INT16>(16, 16, (INT16)nNewPosY);

	xDefSubclassProc(WM_MOVE, 0, xm_RawLParam);
};
void WindowBaseClass::xOnSize(int nType, unsigned int uNewWidth, unsigned int uNewHeight){
	// 与 OnMove() 类似，两个参数事实上是从 wParam 中取高低 16 位扩展而来的。
	// 如果要获得窗体大小尽量使用 GetClientRect()。
	xm_RawLParam.Combine<UINT16>(0, 16, (UINT16)uNewWidth);
	xm_RawLParam.Combine<UINT16>(16, 16, (UINT16)uNewHeight);

	xDefSubclassProc(WM_SIZE, (WPARAM)nType, xm_RawLParam);
};
bool WindowBaseClass::xOnCommand(int nControlID, WORD wNotifyCode, HWND hSender){
	const auto iterMapForControl = xm_mapOnCommandProcs.find(nControlID);
	if(iterMapForControl != xm_mapOnCommandProcs.end()){
		const auto iterProc = iterMapForControl->second.find(wNotifyCode);
		if(iterProc != iterMapForControl->second.end()){
			(this->*(iterProc->second))(hSender);
			return true;
		}

		const auto iterGeneric = iterMapForControl->second.find(DONT_CARE);
		if(iterGeneric != iterMapForControl->second.end()){
			(this->*(iterGeneric->second))(hSender);
			return true;
		}
	}

	xm_RawWParam.Combine<INT16>(0, 16, (INT16)nControlID);
	xm_RawWParam.Combine<WORD>(16, 16, (WORD)wNotifyCode);

	return xDefSubclassProc(WM_COMMAND, xm_RawWParam, (LPARAM)hSender) != 0;
};
bool WindowBaseClass::xOnSysCommand(int nCommand, long lCurPosX, long lCurPosY_Type){
	return xDefSubclassProc(WM_SYSCOMMAND, (WPARAM)nCommand, MAKELPARAM(lCurPosX, lCurPosY_Type)) != 0;
};
bool WindowBaseClass::xOnNotify(UINT uCode, UINT_PTR uSenderID, HWND hSender, const NMHDR *pNMHdr){
	UNREF_PARAM(uCode);
	UNREF_PARAM(hSender);

	const auto iterProc = xm_mapOnNotifyProcs.find((int)uSenderID);
	if(iterProc != xm_mapOnNotifyProcs.end()){
		(this->*(iterProc->second))(hSender, pNMHdr);
		return true;
	}

	return xDefSubclassProc(WM_NOTIFY, uSenderID, (LPARAM)pNMHdr) != 0;
};
void WindowBaseClass::xOnTimer(UINT_PTR uTimerID, TIMERPROC pfnTimerProc){
	xDefSubclassProc(WM_TIMER, uTimerID, (LPARAM)pfnTimerProc);
}
void WindowBaseClass::xOnKeyDown(int nVirtualKeyCode, WORD wRepeatCount, BYTE byScanCode, bool bIsExtended, bool bIsPreviouslyDown){
	xm_RawLParam.Combine<WORD>(0, 16, wRepeatCount);
	xm_RawLParam.Combine<BYTE>(16, 8, byScanCode);
	xm_RawLParam.Combine<bool>(24, 1, bIsExtended);
	xm_RawLParam.Combine<bool>(30, 1, bIsPreviouslyDown);

	xDefSubclassProc(WM_KEYDOWN, (WPARAM)nVirtualKeyCode, xm_RawLParam);
}
void WindowBaseClass::xOnKeyUp(int nVirtualKeyCode, BYTE byScanCode, bool bIsExtended){
	xm_RawLParam.Combine<BYTE>(16, 8, byScanCode);
	xm_RawLParam.Combine<bool>(24, 1, bIsExtended);

	xDefSubclassProc(WM_KEYUP, (WPARAM)nVirtualKeyCode, xm_RawLParam);
}
void WindowBaseClass::xOnSysKeyDown(int nVirtualKeyCode, WORD wRepeatCount, BYTE byScanCode, bool bIsExtended, bool bIsIsloated, bool bIsPreviouslyDown){
	xm_RawLParam.Combine<WORD>(0, 16, wRepeatCount);
	xm_RawLParam.Combine<BYTE>(16, 8, byScanCode);
	xm_RawLParam.Combine<bool>(24, 1, bIsExtended);
	xm_RawLParam.Combine<bool>(29, 1, bIsIsloated);
	xm_RawLParam.Combine<bool>(30, 1, bIsPreviouslyDown);

	xDefSubclassProc(WM_SYSKEYDOWN, (WPARAM)nVirtualKeyCode, xm_RawLParam);
}
void WindowBaseClass::xOnSysKeyUp(int nVirtualKeyCode, BYTE byScanCode, bool bIsIsloated, bool bIsExtended){
	xm_RawLParam.Combine<BYTE>(16, 8, byScanCode);
	xm_RawLParam.Combine<bool>(24, 1, bIsExtended);
	xm_RawLParam.Combine<bool>(29, 1, bIsIsloated);

	xDefSubclassProc(WM_SYSKEYUP, (WPARAM)nVirtualKeyCode, xm_RawLParam);
}
void WindowBaseClass::xOnLButtonDown(bool IsCtrlDown, bool bIsShiftDown, bool bIsMButtonDown, bool bIsRButtonDown, long lXPos, long lYPos){
	xm_RawWParam.CombineBool(MK_CONTROL, IsCtrlDown);
	xm_RawWParam.CombineBool(MK_SHIFT, bIsShiftDown);
	xm_RawWParam.CombineBool(MK_MBUTTON, bIsMButtonDown);
	xm_RawWParam.CombineBool(MK_RBUTTON, bIsRButtonDown);
	xm_RawLParam.Combine<INT16>(0, 16, (INT16)lXPos);
	xm_RawLParam.Combine<INT16>(16, 16, (INT16)lYPos);

	xDefSubclassProc(WM_LBUTTONDOWN, xm_RawWParam, xm_RawLParam);
}
void WindowBaseClass::xOnLButtonUp(bool IsCtrlDown, bool bIsShiftDown, bool bIsMButtonDown, bool bIsRButtonDown, long lXPos, long lYPos){
	xm_RawWParam.CombineBool(MK_CONTROL, IsCtrlDown);
	xm_RawWParam.CombineBool(MK_SHIFT, bIsShiftDown);
	xm_RawWParam.CombineBool(MK_MBUTTON, bIsMButtonDown);
	xm_RawWParam.CombineBool(MK_RBUTTON, bIsRButtonDown);
	xm_RawLParam.Combine<INT16>(0, 16, (INT16)lXPos);
	xm_RawLParam.Combine<INT16>(16, 16, (INT16)lYPos);

	xDefSubclassProc(WM_LBUTTONUP, xm_RawWParam, xm_RawLParam);
}
void WindowBaseClass::xOnRButtonDown(bool IsCtrlDown, bool bIsShiftDown, bool bIsLButtonDown, bool bIsMButtonDown, long lXPos, long lYPos){
	xm_RawWParam.CombineBool(MK_CONTROL, IsCtrlDown);
	xm_RawWParam.CombineBool(MK_SHIFT, bIsShiftDown);
	xm_RawWParam.CombineBool(MK_LBUTTON, bIsLButtonDown);
	xm_RawWParam.CombineBool(MK_MBUTTON, bIsMButtonDown);
	xm_RawLParam.Combine<INT16>(0, 16, (INT16)lXPos);
	xm_RawLParam.Combine<INT16>(16, 16, (INT16)lYPos);

	xDefSubclassProc(WM_RBUTTONDOWN, xm_RawWParam, xm_RawLParam);
}
void WindowBaseClass::xOnRButtonUp(bool IsCtrlDown, bool bIsShiftDown, bool bIsLButtonDown, bool bIsMButtonDown, long lXPos, long lYPos){
	xm_RawWParam.CombineBool(MK_CONTROL, IsCtrlDown);
	xm_RawWParam.CombineBool(MK_SHIFT, bIsShiftDown);
	xm_RawWParam.CombineBool(MK_LBUTTON, bIsLButtonDown);
	xm_RawWParam.CombineBool(MK_MBUTTON, bIsMButtonDown);
	xm_RawLParam.Combine<INT16>(0, 16, (INT16)lXPos);
	xm_RawLParam.Combine<INT16>(16, 16, (INT16)lYPos);

	xDefSubclassProc(WM_RBUTTONUP, xm_RawWParam, xm_RawLParam);
}
void WindowBaseClass::xOnMButtonDown(bool IsCtrlDown, bool bIsShiftDown, bool bIsLButtonDown, bool bIsRButtonDown, long lXPos, long lYPos){
	xm_RawWParam.CombineBool(MK_CONTROL, IsCtrlDown);
	xm_RawWParam.CombineBool(MK_SHIFT, bIsShiftDown);
	xm_RawWParam.CombineBool(MK_LBUTTON, bIsLButtonDown);
	xm_RawWParam.CombineBool(MK_RBUTTON, bIsRButtonDown);
	xm_RawLParam.Combine<INT16>(0, 16, (INT16)lXPos);
	xm_RawLParam.Combine<INT16>(16, 16, (INT16)lYPos);

	xDefSubclassProc(WM_MBUTTONDOWN, xm_RawWParam, xm_RawLParam);
}
void WindowBaseClass::xOnMButtonUp(bool IsCtrlDown, bool bIsShiftDown, bool bIsLButtonDown, bool bIsRButtonDown, long lXPos, long lYPos){
	xm_RawWParam.CombineBool(MK_CONTROL, IsCtrlDown);
	xm_RawWParam.CombineBool(MK_SHIFT, bIsShiftDown);
	xm_RawWParam.CombineBool(MK_LBUTTON, bIsLButtonDown);
	xm_RawWParam.CombineBool(MK_RBUTTON, bIsRButtonDown);
	xm_RawLParam.Combine<INT16>(0, 16,(INT16) lXPos);
	xm_RawLParam.Combine<INT16>(16, 16, (INT16)lYPos);

	xDefSubclassProc(WM_MBUTTONUP, xm_RawWParam, xm_RawLParam);
}
bool WindowBaseClass::xOnSetCursor(HWND hInWhichWindow, int nHitTestCode, UINT uMsg){
	xm_RawLParam.Combine<INT16>(0, 16, (INT16)nHitTestCode);
	xm_RawLParam.Combine<UINT16>(16, 16, (UINT16)uMsg);

	return xDefSubclassProc(WM_SETCURSOR, (WPARAM)hInWhichWindow, xm_RawLParam) != FALSE;
}

void WindowBaseClass::Subclass(HWND hWnd){
	Unsubclass();

	if(hWnd == NULL){
		return;
	}

	ASSERT(xm_pfnOldWndProc == nullptr);
	ASSERT(xm_hWnd == NULL);
	ASSERT(xm_pPrevSubclasser == nullptr);
	ASSERT(xm_pNextSubclasser == nullptr);

	ASSERT(::IsWindow(hWnd));

	WindowBaseClass *const pOldSubclasser = (WindowBaseClass *)::GetWindowLongPtr(xm_hWnd, GWLP_USERDATA);
	if(pOldSubclasser == nullptr){
		// 如果是首个进行子类化操作的对象，简单子类化即可。
		xm_hWnd = hWnd;

		::SetWindowLongPtr(xm_hWnd, GWLP_USERDATA, (LONG_PTR)this);
		xm_pfnOldWndProc = (WNDPROC)::SetWindowLongPtr(xm_hWnd, GWLP_WNDPROC, (LONG_PTR)&xWndProc);
	} else {
		// 否则，建立双向链表。这个链表总是在头部插入的。
		ASSERT(pOldSubclasser->xm_pPrevSubclasser == nullptr);

		SubclassBefore(*pOldSubclasser);

		ASSERT(xm_hWnd == hWnd);
	}

	xPostSubclass();
}
void WindowBaseClass::SubclassBefore(WindowBaseClass &which){
	Unsubclass();

	ASSERT(xm_pfnOldWndProc == nullptr);
	ASSERT(xm_hWnd == NULL);
	ASSERT(xm_pPrevSubclasser == nullptr);
	ASSERT(xm_pNextSubclasser == nullptr);

	xm_hWnd = which.xm_hWnd;
	ASSERT(::IsWindow(xm_hWnd));

	xm_pPrevSubclasser = which.xm_pPrevSubclasser;
	xm_pNextSubclasser = &which;

	if(xm_pPrevSubclasser != nullptr){
		xm_pPrevSubclasser->xm_pNextSubclasser = this;
	}
	which.xm_pPrevSubclasser = this;

	// 如果在双向链表头部插入，需要将消息响应实例指向新的头部。
	if(xm_pPrevSubclasser == nullptr){
		::SetWindowLongPtr(xm_hWnd, GWLP_USERDATA, (LONG_PTR)this);
	}
}
void WindowBaseClass::SubclassAfter(WindowBaseClass &which){
	Unsubclass();

	ASSERT(xm_pfnOldWndProc == nullptr);
	ASSERT(xm_hWnd == NULL);
	ASSERT(xm_pPrevSubclasser == nullptr);
	ASSERT(xm_pNextSubclasser == nullptr);

	xm_hWnd = which.xm_hWnd;
	ASSERT(::IsWindow(xm_hWnd));

	xm_pfnOldWndProc = which.xm_pfnOldWndProc;
	which.xm_pfnOldWndProc = nullptr;

	xm_pPrevSubclasser = &which;
	xm_pNextSubclasser = which.xm_pNextSubclasser;

	which.xm_pNextSubclasser = this;
	if(xm_pNextSubclasser != nullptr){
		xm_pNextSubclasser->xm_pPrevSubclasser = this;
	}
}
void WindowBaseClass::Unsubclass(){
	if(xm_hWnd == NULL){
		return;
	}

	xPreUnsubclass();

	ASSERT(::IsWindow(xm_hWnd));

	if(xm_pPrevSubclasser == nullptr){
		if(xm_pNextSubclasser == nullptr){
			ASSERT(xm_pfnOldWndProc != nullptr);
			::SetWindowLongPtr(xm_hWnd, GWLP_WNDPROC, (LONG_PTR)xm_pfnOldWndProc);
			xm_pfnOldWndProc = nullptr;

			::SetWindowLongPtr(xm_hWnd, GWLP_USERDATA, (LONG_PTR)nullptr);
		} else {
			::SetWindowLongPtr(xm_hWnd, GWLP_USERDATA, (LONG_PTR)xm_pNextSubclasser);
		}
	} else {
		xm_pPrevSubclasser->xm_pfnOldWndProc = xm_pfnOldWndProc;
		xm_pfnOldWndProc = nullptr;

		xm_pPrevSubclasser->xm_pNextSubclasser = xm_pNextSubclasser;
	}
	if(xm_pNextSubclasser != nullptr){
		xm_pNextSubclasser->xm_pPrevSubclasser = xm_pPrevSubclasser;
	}
	xm_pPrevSubclasser = nullptr;
	xm_pNextSubclasser = nullptr;

	xm_hWnd = NULL;

	ASSERT(xm_pfnOldWndProc == nullptr);
	ASSERT(xm_hWnd == NULL);
	ASSERT(xm_pPrevSubclasser == nullptr);
	ASSERT(xm_pNextSubclasser == nullptr);
}

// 供其他类调用的一些接口。
HWND WindowBaseClass::GetHWnd() const {
	return xm_hWnd;
}
HWND WindowBaseClass::GetParentHWnd() const {
	if(xm_hWnd == NULL){
		return NULL;
	}
	return ::GetAncestor(xm_hWnd, GA_PARENT);
}
WindowBaseClass *WindowBaseClass::GetParent() const {
	return GetWindowPtr(GetParentHWnd());
}

int WindowBaseClass::GetDlgCtrlID() const {
	ASSERT(xm_hWnd != NULL);

	return ::GetDlgCtrlID(xm_hWnd);
}

bool WindowBaseClass::IsVisible() const {
	ASSERT(xm_hWnd != NULL);

	return ::IsWindowVisible(xm_hWnd) != FALSE;
}
void WindowBaseClass::Show(int nCmdShow){
	ASSERT(xm_hWnd != NULL);

	::ShowWindow(xm_hWnd, nCmdShow);
}
bool WindowBaseClass::IsEnabled() const {
	ASSERT(xm_hWnd != NULL);

	return ::IsWindowEnabled(xm_hWnd) != FALSE;
}
void WindowBaseClass::Enable(bool bToEnable){
	ASSERT(xm_hWnd != NULL);

	::EnableWindow(xm_hWnd, bToEnable ? TRUE : FALSE);
}
std::size_t WindowBaseClass::GetTextLength() const {
	ASSERT(xm_hWnd != NULL);

	return (std::size_t)::GetWindowTextLength(xm_hWnd);
}
TString WindowBaseClass::GetText() const {
	ASSERT(xm_hWnd != NULL);

	TString strRet;
	const std::size_t uTextLength = GetTextLength();
	strRet.Resize(uTextLength);
	::GetWindowText(xm_hWnd, (LPTSTR)strRet, (int)(uTextLength + 1));
	return std::move(strRet);
}
void WindowBaseClass::SetText(LPCTSTR pszSetToWhich){
	ASSERT(xm_hWnd != NULL);

	::SetWindowText(xm_hWnd, pszSetToWhich);
}
void WindowBaseClass::SetRedraw(bool bToSet, bool bInvalidateIfSetToTrue){
	ASSERT(xm_hWnd != NULL);

	SendMessage(WM_SETREDRAW, (WPARAM)(bToSet ? TRUE : FALSE));
	if(bInvalidateIfSetToTrue){
		Invalidate();
	}
}
void WindowBaseClass::Validate() const {
	ASSERT(xm_hWnd != NULL);

	::ValidateRect(xm_hWnd, nullptr);
}
void WindowBaseClass::Invalidate(bool bErase) const {
	ASSERT(xm_hWnd != NULL);

	::InvalidateRect(xm_hWnd, nullptr, bErase ? TRUE : FALSE);
}
void WindowBaseClass::Update() const {
	ASSERT(xm_hWnd != NULL);

	::UpdateWindow(xm_hWnd);
}

DWORD WindowBaseClass::GetStyle() const {
	return (DWORD)::GetWindowLongPtr(xm_hWnd, GWL_STYLE);
}
void WindowBaseClass::SetStyle(DWORD dwMask, DWORD dwValue){
	ASSERT(xm_hWnd != NULL);

	::SetWindowLongPtr(xm_hWnd, GWL_STYLE, (LONG_PTR)(((GetStyle() ^ dwValue) & ~dwMask) ^ dwValue));
}
DWORD WindowBaseClass::GetExStyle() const {
	return (DWORD)::GetWindowLongPtr(xm_hWnd, GWL_EXSTYLE);
}
void WindowBaseClass::SetExStyle(DWORD dwMask, DWORD dwValue){
	ASSERT(xm_hWnd != NULL);

	::SetWindowLongPtr(xm_hWnd, GWL_EXSTYLE, (LONG_PTR)(((GetExStyle() ^ dwValue) & ~dwMask) ^ dwValue));
}

RECT WindowBaseClass::GetAbsRect() const {
	ASSERT(xm_hWnd != NULL);

	RECT rect;
	::GetWindowRect(xm_hWnd, &rect);
	return std::move(rect);
}
RECT WindowBaseClass::GetRect() const {
	ASSERT(xm_hWnd != NULL);

	const HWND hParent = GetParentHWnd();
	if(hParent == NULL){
		return GetAbsRect();
	}

	RECT rect;
	::GetWindowRect(xm_hWnd, &rect);

	POINT apntTemp[2];

	apntTemp[0].x = rect.left;
	apntTemp[0].y = rect.top;
	apntTemp[1].x = rect.right;
	apntTemp[1].y = rect.bottom;

	::MapWindowPoints(HWND_DESKTOP, hParent, apntTemp, COUNTOF(apntTemp));

	rect.left	= apntTemp[0].x;
	rect.top	= apntTemp[0].y;
	rect.right	= apntTemp[1].x;
	rect.bottom	= apntTemp[1].y;

	return std::move(rect);
}
RECT WindowBaseClass::GetChildRect(HWND hChild) const {
	ASSERT(xm_hWnd != NULL);
	ASSERT(hChild != NULL);

	RECT rect;
	::GetWindowRect(hChild, &rect);

	POINT apntTemp[2];

	apntTemp[0].x = rect.left;
	apntTemp[0].y = rect.top;
	apntTemp[1].x = rect.right;
	apntTemp[1].y = rect.bottom;

	::MapWindowPoints(HWND_DESKTOP, xm_hWnd, apntTemp, COUNTOF(apntTemp));

	rect.left	= apntTemp[0].x;
	rect.top	= apntTemp[0].y;
	rect.right	= apntTemp[1].x;
	rect.bottom	= apntTemp[1].y;

	return std::move(rect);
}
RECT WindowBaseClass::GetClientAbsRect() const{
	ASSERT(xm_hWnd != NULL);

	RECT rect;
	::GetClientRect(xm_hWnd, &rect);

	POINT apntTemp[2];

	apntTemp[0].x = rect.left;
	apntTemp[0].y = rect.top;
	apntTemp[1].x = rect.right;
	apntTemp[1].y = rect.bottom;

	::MapWindowPoints(xm_hWnd, HWND_DESKTOP, apntTemp, COUNTOF(apntTemp));

	rect.left	= apntTemp[0].x;
	rect.top	= apntTemp[0].y;
	rect.right	= apntTemp[1].x;
	rect.bottom	= apntTemp[1].y;

	return std::move(rect);
}
RECT WindowBaseClass::GetClientRect() const{
	ASSERT(xm_hWnd != NULL);

	RECT rect;
	::GetClientRect(xm_hWnd, &rect);
	return std::move(rect);
}
void WindowBaseClass::SetAbsRect(const RECT &rect){
	ASSERT(xm_hWnd != NULL);

	const HWND hParent = GetParentHWnd();
	if(hParent == NULL){
		return SetRect(rect);
	}

	POINT pntUpperLeft = { rect.left, rect.top };
	::ScreenToClient(hParent, &pntUpperLeft);

	::SetWindowPos(xm_hWnd, NULL, (int)pntUpperLeft.x, (int)pntUpperLeft.y, (int)(rect.right - rect.left), (int)(rect.bottom - rect.top), SWP_NOACTIVATE | SWP_NOZORDER);
}
void WindowBaseClass::SetRect(const RECT &rect){
	ASSERT(xm_hWnd != NULL);

	::SetWindowPos(xm_hWnd, NULL, (int)rect.left, (int)rect.top, (int)(rect.right - rect.left), (int)(rect.bottom - rect.top), SWP_NOACTIVATE | SWP_NOZORDER);
}
void WindowBaseClass::CenterIntoAbsRect(const RECT &rect){
	ASSERT(xm_hWnd != NULL);

	const HWND hParent = GetParentHWnd();
	if(hParent == NULL){
		return CenterIntoRect(rect);
	}

	RECT CurrentRect;
	::GetWindowRect(xm_hWnd, &CurrentRect);
	const long lWindowWidth = CurrentRect.right - CurrentRect.left;
	const long lWindowHeight = CurrentRect.bottom - CurrentRect.top;

	POINT pntUpperLeft = { rect.left, rect.top };
	::ScreenToClient(hParent, &pntUpperLeft);

	::SetWindowPos(xm_hWnd, NULL, (int)(pntUpperLeft.x + ((rect.right - rect.left) - lWindowWidth) / 2), (int)(pntUpperLeft.y + ((rect.bottom - rect.top) - lWindowHeight) / 2), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}
void WindowBaseClass::CenterIntoRect(const RECT &rect){
	ASSERT(xm_hWnd != NULL);

	RECT CurrentRect;
	::GetWindowRect(xm_hWnd, &CurrentRect);
	const long lWindowWidth = CurrentRect.right - CurrentRect.left;
	const long lWindowHeight = CurrentRect.bottom - CurrentRect.top;

	::SetWindowPos(xm_hWnd, NULL, (int)((rect.left + rect.right - lWindowWidth) / 2), (int)((rect.top + rect.bottom - lWindowHeight) / 2), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

LRESULT WindowBaseClass::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) const {
	ASSERT(xm_hWnd != NULL);

	return ::SendMessage(xm_hWnd, uMsg, wParam, lParam);
}
void WindowBaseClass::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) const {
	ASSERT(xm_hWnd != NULL);

	::PostMessage(xm_hWnd, uMsg, wParam, lParam);
}
