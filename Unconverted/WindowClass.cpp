// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "WindowClass.hpp"
using namespace MCF;

// 静态成员变量。
std::atomic<unsigned int>			WindowClass::xs_uClassIndex			(0);
TLSManagerTemplate<WindowClass *>	WindowClass::xs_tlsCurrentInstance;

// 静态成员函数。
LRESULT __stdcall WindowClass::xWindowProcStub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	WindowClass **const ppCurrentInstance = xs_tlsCurrentInstance;
	WindowClass *const pCurrentInstance = *ppCurrentInstance;
	*ppCurrentInstance = nullptr;
	ASSERT(pCurrentInstance != nullptr);

	::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)&::DefWindowProc);
	pCurrentInstance->Subclass(hWnd);

	return pCurrentInstance->SendMessage(uMsg, wParam, lParam);
}

// 构造函数和析构函数。
WindowClass::WindowClass(
	HINSTANCE			hInstance,
	HICON				hIcon,
	HICON				hSmallIcon,
	HCURSOR				hCursor,
	HBRUSH				hBackground,
	LPCTSTR				pszMenu,
	DWORD				dwClassStyle,
	DWORD				dwStyle,
	DWORD				dwExStyle,
	const std::size_t	*puStatusBarWidths,
	std::size_t			uStatusBarPartCount,
	std::size_t			uStatusBarMinHeight
) :
	StandaloneWindowClass	(puStatusBarWidths, uStatusBarPartCount, uStatusBarMinHeight),
	xm_hInstance			(hInstance),
	xm_dwStyle				(dwStyle),
	xm_dwExStyle			(dwExStyle)
{
	TString strClassName;
	strClassName.Format(_T("__MCF_WNDCLASSEX_%08X__"), ++xs_uClassIndex);

	WNDCLASSEX WndClassEx;
	WndClassEx.cbSize			= sizeof(WndClassEx);
	WndClassEx.style			= dwClassStyle;
	WndClassEx.lpfnWndProc		= &xWindowProcStub;
	WndClassEx.cbClsExtra		= 0;
	WndClassEx.cbWndExtra		= 0;
	WndClassEx.hInstance		= hInstance;
	WndClassEx.hIcon			= hIcon;
	WndClassEx.hCursor			= hCursor;
	WndClassEx.hbrBackground	= hBackground;
	WndClassEx.lpszMenuName		= pszMenu;
	WndClassEx.lpszClassName	= strClassName;
	WndClassEx.hIconSm			= hSmallIcon;

	xm_atmWindowClass = VERIFY(::RegisterClassEx(&WndClassEx));
}
WindowClass::~WindowClass(){
	// 必须手工调用 Destroy()，或者由系统自动销毁（如果是子窗口）。
	ASSERT(GetHWnd() == NULL);

	::UnregisterClass(MAKEINTATOM(xm_atmWindowClass), xm_hInstance);
}

// 其他非静态成员函数。
LRESULT WindowClass::xOnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam){
	if(uMsg == WM_CREATE){
		return xOnCreate((LPCREATESTRUCT)lParam) ? 0 : (LRESULT)-1;
	}
	return StandaloneWindowClass::xOnMessage(uMsg, wParam, lParam);
}

bool WindowClass::xOnCreate(LPCREATESTRUCT pCreateStruct){
	if(StandaloneWindowClass::xOnMessage(WM_CREATE, 0, (LPARAM)pCreateStruct) == -1){
		return false;
	}
	xOnInitialize();
	return true;
}

void WindowClass::Create(LPCTSTR pszCaption, int x, int y, int nWidth, int nHeight, HWND hParent, LPVOID pParam, int nChildID){
	Destroy();

	WindowClass **const ppCurrentInstance = xs_tlsCurrentInstance;
	ASSERT(*ppCurrentInstance == nullptr);
	*ppCurrentInstance = this;

	const HWND hCreatedWindow = VERIFY(::CreateWindowEx(
		xm_dwExStyle,
		MAKEINTATOM(xm_atmWindowClass),
		pszCaption,
		xm_dwStyle,
		x,
		y,
		nWidth,
		nHeight,
		hParent,
		((xm_dwStyle & WS_CHILD) == 0) ? NULL : (HMENU)nChildID,
		xm_hInstance,
		pParam
	));

	UNREF_PARAM(hCreatedWindow);

	ASSERT(GetHWnd() == hCreatedWindow);
}
void WindowClass::Destroy(){
	if(GetHWnd() != NULL){
		::DestroyWindow(GetHWnd());

		ASSERT(GetHWnd() == NULL);
	}
}
