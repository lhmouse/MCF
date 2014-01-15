// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_WINDOW_CLASS_HPP__
#define __MCF_WINDOW_CLASS_HPP__

#include "StdMCF.hpp"
#include "Memory.hpp"
#include "StandaloneWindowClass.hpp"

namespace MCF {
	class WindowClass : public StandaloneWindowClass {
	private:
		static std::atomic<unsigned int> xs_uClassIndex;
		static TLSManagerTemplate<WindowClass *> xs_tlsCurrentInstance;
	private:
		static LRESULT __stdcall xWindowProcStub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		const HINSTANCE xm_hInstance;
		const DWORD xm_dwStyle;
		const DWORD xm_dwExStyle;

		ATOM xm_atmWindowClass;
	public:
		WindowClass(
			HINSTANCE			hInstance,
			HICON				hIcon,
			HICON				hSmallIcon,
			HCURSOR				hCursor,
			HBRUSH				hBackground,
			LPCTSTR				pszMenu,
			DWORD				dwClassStyle,
			DWORD				dwStyle,
			DWORD				dwExStyle,
			const std::size_t	*puStatusBarWidths = nullptr,
			std::size_t			uStatusBarPartCount = 0,
			std::size_t			uStatusBarMinHeight = 0
		);
		virtual ~WindowClass();
	protected:
		virtual LRESULT xOnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

		virtual bool xOnCreate(LPCREATESTRUCT pCreateStruct);
	public:
		HINSTANCE GetHInstance() const;

		void Create(LPCTSTR pszCaption, int x, int y, int nWidth, int nHeight, HWND hParent, LPVOID pParam, int nChildID = -1);
		void Destroy();
	};
}

#endif
