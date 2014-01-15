// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_WINDOW_BASE_CLASS_HPP__
#define __MCF_WINDOW_BASE_CLASS_HPP__

#include "StdMCF.hpp"
#include "Memory.hpp"
#include <map>

namespace MCF {
	class WindowBaseClass : NO_COPY_OR_ASSIGN {
	public:
		static const DWORD DONT_CARE = 0xFFFFFFFF;
	private:
		typedef void (WindowBaseClass::*xLPFNONCOMMANDPROC)(HWND);
		typedef void (WindowBaseClass::*xLPFNONNOTIFYPROC)(HWND, const NMHDR *);
	private:
		static LRESULT CALLBACK xWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static DWORD xMakeAcceleratorMapKey(bool bCtrlCombined, bool bAltCombined, bool bShiftCombined, char chVKCode);
	public:
		static WindowBaseClass *GetWindowPtr(HWND hWnd);
	private:
		WNDPROC xm_pfnOldWndProc;
		HWND xm_hWnd;
		WindowBaseClass *xm_pPrevSubclasser;
		WindowBaseClass *xm_pNextSubclasser;

		BitFieldTemplate<WPARAM> xm_RawWParam;
		BitFieldTemplate<LPARAM> xm_RawLParam;

		std::map<int, std::map<DWORD, xLPFNONCOMMANDPROC>> xm_mapOnCommandProcs;
		std::map<int, xLPFNONNOTIFYPROC> xm_mapOnNotifyProcs;
	public:
		WindowBaseClass();
		virtual ~WindowBaseClass();
	private:
		LRESULT xDefSubclassProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void xDestroySubclassChain();

		void xDoSetOnCommand(int nControlID, DWORD dwNotifyCode, xLPFNONCOMMANDPROC pfnOnCommandProc);
		void xDoSetOnNotify(int nControlID, xLPFNONNOTIFYPROC pfnOnNotifyProc);
	protected:
		virtual void xPostSubclass();
		virtual void xPreUnsubclass();

		virtual LRESULT xOnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

		// xOnInitialize 并不是一个消息响应函数，而是一个桩。
		virtual void xOnInitialize();
		virtual void xOnDestroy();
		virtual void xOnClose();
		virtual void xOnShow(bool bToShow, int nStatus);
		virtual bool xOnEraseBkgnd(HDC hDC);
		virtual void xOnPaint();
		virtual void xOnMove(int nNewPosX, int nNewPosY);
		virtual void xOnSize(int nType, unsigned int uNewWidth, unsigned int uNewHeight);
		virtual bool xOnCommand(int nControlID, WORD wNotifyCode, HWND hSender);
		virtual bool xOnSysCommand(int nCommand, long lCurPosX, long lCurPosY_Type);
		virtual bool xOnNotify(UINT uCode, UINT_PTR uSenderID, HWND hSender, const NMHDR *pNMHdr);
		virtual void xOnTimer(UINT_PTR uTimerID, TIMERPROC pfnTimerProc);
		virtual void xOnKeyDown(int nVirtualKeyCode, WORD wRepeatCount, BYTE byScanCode, bool bIsExtended, bool bIsPreviouslyDown);
		virtual void xOnKeyUp(int nVirtualKeyCode, BYTE byScanCode, bool bIsExtended);
		virtual void xOnSysKeyDown(int nVirtualKeyCode, WORD wRepeatCount, BYTE byScanCode, bool bIsExtended, bool bIsIsloated, bool bIsPreviouslyDown);
		virtual void xOnSysKeyUp(int nVirtualKeyCode, BYTE byScanCode, bool bIsIsloated, bool bIsExtended);
		virtual void xOnLButtonDown(bool IsCtrlDown, bool bIsShiftDown, bool bIsMButtonDown, bool bIsRButtonDown, long lXPos, long lYPos);
		virtual void xOnLButtonUp(bool IsCtrlDown, bool bIsShiftDown, bool bIsMButtonDown, bool bIsRButtonDown, long lXPos, long lYPos);
		virtual void xOnRButtonDown(bool IsCtrlDown, bool bIsShiftDown, bool bIsLButtonDown, bool bIsMButtonDown, long lXPos, long lYPos);
		virtual void xOnRButtonUp(bool IsCtrlDown, bool bIsShiftDown, bool bIsLButtonDown, bool bIsMButtonDown, long lXPos, long lYPos);
		virtual void xOnMButtonDown(bool IsCtrlDown, bool bIsShiftDown, bool bIsLButtonDown, bool bIsRButtonDown, long lXPos, long lYPos);
		virtual void xOnMButtonUp(bool IsCtrlDown, bool bIsShiftDown, bool bIsLButtonDown, bool bIsRButtonDown, long lXPos, long lYPos);
		virtual bool xOnSetCursor(HWND hInWhichWindow, int nHitTestCode, UINT uMsg);
	public:
		void Subclass(HWND hWnd);
		void SubclassBefore(WindowBaseClass &which);
		void SubclassAfter(WindowBaseClass &which);
		void Unsubclass();

		HWND GetHWnd() const;
		HWND GetParentHWnd() const;
		WindowBaseClass *GetParent() const;

		int GetDlgCtrlID() const;

		bool IsVisible() const;
		void Show(int nCmdShow);
		bool IsEnabled() const;
		void Enable(bool bToEnable);
		std::size_t GetTextLength() const;
		TString GetText() const;
		void SetText(LPCTSTR pszSetToWhich);
		void SetRedraw(bool bToSet, bool bInvalidateIfSetToTrue = true);
		void Validate() const;
		void Invalidate(bool bErase = true) const;
		void Update() const;

		DWORD GetStyle() const;
		void SetStyle(DWORD dwMask, DWORD dwValue);
		DWORD GetExStyle() const;
		void SetExStyle(DWORD dwMask, DWORD dwValue);

		RECT GetAbsRect() const;
		RECT GetRect() const;
		RECT GetChildRect(HWND hChild) const;
		RECT GetClientAbsRect() const;
		RECT GetClientRect() const;
		void SetAbsRect(const RECT &rect);
		void SetRect(const RECT &rect);
		void CenterIntoAbsRect(const RECT &rect);
		void CenterIntoRect(const RECT &rect);

		LRESULT SendMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) const;
		void PostMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) const;

		template<class WINDOW_TYPE>
		inline __always_inline void SetOnCommand(int nControlID, DWORD dwNotifyCode, void (WINDOW_TYPE::*pfnOnCommandProc)(HWND)){
			xDoSetOnCommand(nControlID, dwNotifyCode, (std::enable_if<std::is_base_of<WindowBaseClass, typename WINDOW_TYPE>::value, xLPFNONCOMMANDPROC>::type)pfnOnCommandProc);
		}
		template<class WINDOW_TYPE, typename NMHDR_TYPE>
		inline __always_inline void SetOnNotify(int nControlID, void (WINDOW_TYPE::*pfnOnNotifyProc)(HWND, const NMHDR_TYPE *)){
			xDoSetOnNotify(nControlID, (std::enable_if<std::is_base_of<WindowBaseClass, typename WINDOW_TYPE>::value, xLPFNONNOTIFYPROC>::type)pfnOnNotifyProc);
		}
	};
}

#endif
