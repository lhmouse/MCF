// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_STANDALONE_WINDOW_CLASS_HPP__
#define __MCF_STANDALONE_WINDOW_CLASS_HPP__

#include "StdMCF.hpp"
#include <vector>
#include "WindowBaseClass.hpp"

namespace MCF {
	class StandaloneWindowClass : public WindowBaseClass {
	private:
		class xThreadParasiteClass;
		class xStatusBarClass;
	private:
		xThreadParasiteClass *xm_pThreadParasite;

		xStatusBarClass *xm_pStatusBar;
		std::vector<std::size_t> xm_vecuStatusBarWidths;
		std::size_t xm_uStatusBarMinHeight;
	public:
		StandaloneWindowClass(const std::size_t *puStatusBarWidths, std::size_t uStatusBarPartCount, std::size_t uStatusBarMinHeight);
		virtual ~StandaloneWindowClass();
	protected:
		virtual void xPreUnsubclass() override;

		virtual bool xIdleLoop();
		virtual bool xPreTranslateAccelerator(MSG *pMsg);
		virtual bool xPreTranslateDialogMessage(MSG *pMsg);
		virtual bool xPreTranslateMessage(MSG *pMsg);
		virtual void xPostDispatchMessage(const MSG *pMsg);

		virtual void xOnInitialize() override;
		virtual void xOnDestroy() override;
		virtual void xOnSize(int nType, unsigned int uNewWidth, unsigned int uNewHeight) override;
	public:
		HWND CreateStatusBar(const std::size_t *puWidths, std::size_t uPartCount, unsigned int uMinHeight);
		void DestroyStatusBar();

		HWND GetStatusBarHWnd() const;
		void SetStatusBarText(std::size_t uPartIndex, LPCTSTR pszText);
		void SetStatusBarIcon(std::size_t uPartIndex, HICON hNewIcon);

		int MessageBox(LPCTSTR pszText, LPCTSTR pszCaption, UINT uType);

		int DoMessageLoop(bool bTranslateDialogMessages, std::intptr_t nContext = 0, HINSTANCE hInstance = NULL, int nAcceleratorsResourceID = 0);
		std::intptr_t GetMessageLoopContext() const;
	};
}

#endif
