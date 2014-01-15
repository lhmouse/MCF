// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "DialogBaseClass.hpp"
#include "xDlgItemPosLockClass.hpp"
using namespace MCF;

// 静态成员变量。
TLSManagerTemplate<DialogBaseClass *> DialogBaseClass::xs_tlsCurrentInstance;

// 静态成员函数。
INT_PTR CALLBACK DialogBaseClass::xDlgProcStub(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam){
	DialogBaseClass **const ppCurrentInstance = xs_tlsCurrentInstance;
	DialogBaseClass *const pCurrentInstance = *ppCurrentInstance;
	*ppCurrentInstance = nullptr;
	ASSERT(pCurrentInstance != nullptr);

	::SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR)&xDlgProc);
	pCurrentInstance->Subclass(hDlg);
	const LONG_PTR lMsgResult = (LONG_PTR)pCurrentInstance->SendMessage(uMsg, wParam, lParam);
	switch(uMsg){
		case WM_CHARTOITEM:
		case WM_COMPAREITEM:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
		case WM_INITDIALOG:
		case WM_QUERYDRAGICON:
		case WM_VKEYTOITEM:
			return (INT_PTR)lMsgResult;
		default:
			::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lMsgResult);
			return TRUE;
	}
}
INT_PTR CALLBACK DialogBaseClass::xDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam){
	UNREF_PARAM(hDlg);
	UNREF_PARAM(uMsg);
	UNREF_PARAM(wParam);
	UNREF_PARAM(lParam);

	return FALSE;
}

// 构造函数和析构函数。
DialogBaseClass::DialogBaseClass(
	HINSTANCE				hInstance,
	int						nTemplateID,
	const DIPLITEM			*pDIPLItems,
	std::size_t				uDIPLItemCount,
	const std::size_t		*puStatusBarPartWidths,
	std::size_t				uStatusBarPartCount,
	std::size_t				uStatusBarMinHeight
) :
	StandaloneWindowClass	(puStatusBarPartWidths, uStatusBarPartCount, uStatusBarMinHeight),
	xm_hInstance			(hInstance),
	xm_nTemplateID			(nTemplateID),
	xm_vecDIPLItems			(pDIPLItems, pDIPLItems + (std::ptrdiff_t)uDIPLItemCount)
{
	xm_pDlgItemPosLock	= nullptr;
}
DialogBaseClass::~DialogBaseClass(){
	ASSERT(GetHWnd() == NULL);

	ASSERT(xm_pDlgItemPosLock == nullptr);
}

// 其他非静态成员函数。
LRESULT DialogBaseClass::xOnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam){
	if(uMsg == WM_INITDIALOG){
		return xOnInitDialog((HWND)wParam, lParam) ? TRUE : FALSE;
	}
	return StandaloneWindowClass::xOnMessage(uMsg, wParam, lParam);
}

bool DialogBaseClass::xOnInitDialog(HWND hDefControl, LPARAM lInitParam){
	const bool bOldRet = StandaloneWindowClass::xOnMessage(WM_INITDIALOG, (WPARAM)hDefControl, lInitParam) != 0;

	if(!xm_vecDIPLItems.empty()){
		xm_pDlgItemPosLock = new xDlgItemPosLockClass(this, xm_vecDIPLItems.data(), xm_vecDIPLItems.size());
		xm_pDlgItemPosLock->Update();
	}

	xOnInitialize();
	return bOldRet;
};
void DialogBaseClass::xOnDestroy(){
	delete xm_pDlgItemPosLock;
	xm_pDlgItemPosLock = nullptr;

	StandaloneWindowClass::xOnDestroy();
}
void DialogBaseClass::xOnSize(int nType, unsigned int uNewWidth, unsigned int uNewHeight){
	StandaloneWindowClass::xOnSize(nType, uNewWidth, uNewHeight);

	if(xm_pDlgItemPosLock != nullptr){
		xm_pDlgItemPosLock->Update();
	}
};

HINSTANCE DialogBaseClass::GetHInstance() const {
	return xm_hInstance;
}
int DialogBaseClass::GetTemplateID() const {
	return xm_nTemplateID;
}
HWND DialogBaseClass::GetDlgItemHWnd(int nControlID) const {
	const HWND hDlg = GetHWnd();

	ASSERT(hDlg != NULL);

	return ::GetDlgItem(hDlg, nControlID);
}

void DialogBaseClass::xMakeSubclassDlgItem(WindowBaseClass &which, int nItemID){
	which.Subclass(VERIFY(GetDlgItemHWnd(nItemID)));
}

bool DialogBaseClass::IsDlgItemVisible(int nDlgItemID) const {
	return ::IsWindowVisible(VERIFY(GetDlgItemHWnd(nDlgItemID))) != FALSE;
}
void DialogBaseClass::ShowDlgItem(int nDlgItemID, bool bToShow){
	::ShowWindow(VERIFY(GetDlgItemHWnd(nDlgItemID)), bToShow ? SW_SHOW : SW_HIDE);
}
bool DialogBaseClass::IsDlgItemEnabled(int nDlgItemID) const {
	return ::IsWindowEnabled(VERIFY(GetDlgItemHWnd(nDlgItemID))) != FALSE;
}
void DialogBaseClass::EnableDlgItem(int nDlgItemID, bool bToEnable){
	::EnableWindow(VERIFY(GetDlgItemHWnd(nDlgItemID)), bToEnable ? TRUE : FALSE);
}
std::size_t DialogBaseClass::GetDlgItemTextLength(int nDlgItemID) const {
	return (std::size_t)::GetWindowTextLength(VERIFY(GetDlgItemHWnd(nDlgItemID)));
}
TString DialogBaseClass::GetDlgItemText(int nDlgItemID) const {
	const HWND hDlgItem = VERIFY(GetDlgItemHWnd(nDlgItemID));

	TString strRet;
	const std::size_t uTextLength = (std::size_t)::GetWindowTextLength(hDlgItem);
	strRet.Resize(uTextLength);
	::GetWindowText(hDlgItem, (LPTSTR)strRet, (int)(uTextLength + 1));
	return std::move(strRet);
}
void DialogBaseClass::SetDlgItemText(int nDlgItemID, LPCTSTR pszSetToWhich){
	::SetWindowText(VERIFY(GetDlgItemHWnd(nDlgItemID)), pszSetToWhich);
}
int DialogBaseClass::IsDlgButtonChecked(int nDlgItemID) const {
	return (int)SendDlgItemMessage(nDlgItemID, BM_GETCHECK);
}
void DialogBaseClass::CheckDlgButton(int nDlgItemID, int nCheck){
	SendDlgItemMessage(nDlgItemID, BM_SETCHECK, (WPARAM)nCheck);
}

RECT DialogBaseClass::GetDlgItemAbsRect(int nDlgItemID) const {
	const HWND hDlgItem = VERIFY(GetDlgItemHWnd(nDlgItemID));

	ASSERT(hDlgItem != NULL);

	RECT rect;
	::GetWindowRect(hDlgItem, &rect);
	return std::move(rect);
}
RECT DialogBaseClass::GetDlgItemRect(int nDlgItemID) const {
	return StandaloneWindowClass::GetChildRect(VERIFY(GetDlgItemHWnd(nDlgItemID)));
}

LRESULT DialogBaseClass::SendDlgItemMessage(int nDlgItemID, UINT uMsg, WPARAM wParam, LPARAM lParam) const {
	return ::SendMessage(VERIFY(GetDlgItemHWnd(nDlgItemID)), uMsg, wParam, lParam);
}
void DialogBaseClass::PostDlgItemMessage(int nDlgItemID, UINT uMsg, WPARAM wParam, LPARAM lParam) const {
	::PostMessage(VERIFY(GetDlgItemHWnd(nDlgItemID)), uMsg, wParam, lParam);
}
