// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "StandaloneWindowClass.hpp"
#include "xThreadParasiteClass.hpp"
#include "xStatusBarClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
StandaloneWindowClass::StandaloneWindowClass(const std::size_t *puStatusBarWidths, std::size_t uStatusBarPartCount, std::size_t uStatusBarMinHeight){
	xm_pThreadParasite = nullptr;

	xm_pStatusBar = nullptr;
	if((puStatusBarWidths != nullptr) && (uStatusBarPartCount != 0)){
		xm_vecuStatusBarWidths.assign(puStatusBarWidths, puStatusBarWidths + (std::ptrdiff_t)uStatusBarPartCount);
	}
	xm_uStatusBarMinHeight = uStatusBarMinHeight;
}
StandaloneWindowClass::~StandaloneWindowClass(){
	delete xm_pThreadParasite;
	delete xm_pStatusBar;
}

// 其他非静态成员函数。
void StandaloneWindowClass::xPreUnsubclass(){
	if(xm_pThreadParasite != nullptr){
		xm_pThreadParasite->QuitMessageLoop(0);
		xm_pThreadParasite = nullptr;
	}

	WindowBaseClass::xPreUnsubclass();
}

bool StandaloneWindowClass::xIdleLoop(){
	return false;
}
bool StandaloneWindowClass::xPreTranslateAccelerator(MSG *pMsg){
	UNREF_PARAM(pMsg);

	return false;
}
bool StandaloneWindowClass::xPreTranslateDialogMessage(MSG *pMsg){
	UNREF_PARAM(pMsg);

	return false;
}
bool StandaloneWindowClass::xPreTranslateMessage(MSG *pMsg){
	UNREF_PARAM(pMsg);

	return false;
}
void StandaloneWindowClass::xPostDispatchMessage(const MSG *pMsg){
	UNREF_PARAM(pMsg);
}

void StandaloneWindowClass::xOnInitialize(){
	WindowBaseClass::xOnInitialize();

	if(!xm_vecuStatusBarWidths.empty()){
		xm_pStatusBar = new xStatusBarClass(this);
		xm_pStatusBar->Update();
	}
}
void StandaloneWindowClass::xOnDestroy(){
	delete xm_pStatusBar;
	xm_pStatusBar = nullptr;

	WindowBaseClass::xOnDestroy();
}
void StandaloneWindowClass::xOnSize(int nType, unsigned int uNewWidth, unsigned int uNewHeight){
	WindowBaseClass::xOnSize(nType, uNewWidth, uNewHeight);

	if(xm_pStatusBar != nullptr){
		xm_pStatusBar->Update();
	}
}

HWND StandaloneWindowClass::GetStatusBarHWnd() const {
	return (xm_pStatusBar == nullptr) ? NULL : xm_pStatusBar->GetHWnd();
}
void StandaloneWindowClass::SetStatusBarText(std::size_t uPartIndex, LPCTSTR pszText){
	ASSERT(xm_pStatusBar != nullptr);

	xm_pStatusBar->SendMessage(SB_SETTEXT, (WPARAM)uPartIndex, (LPARAM)pszText);
}
void StandaloneWindowClass::SetStatusBarIcon(std::size_t uPartIndex, HICON hNewIcon){
	ASSERT(xm_pStatusBar != nullptr);

	xm_pStatusBar->SendMessage(SB_SETICON, (WPARAM)uPartIndex, (LPARAM)hNewIcon);
}

int StandaloneWindowClass::MessageBox(LPCTSTR pszText, LPCTSTR pszCaption, UINT uType){
	const HWND hWnd = GetHWnd();

	ASSERT(hWnd != NULL);

	return ::MessageBox(hWnd, pszText, pszCaption, uType);
}

int StandaloneWindowClass::DoMessageLoop(bool bTranslateDialogMessages, std::intptr_t nContext, HINSTANCE hInstance, int nAcceleratorsResourceID){
	if((hInstance != NULL) && (nAcceleratorsResourceID != 0)){
		return xThreadParasiteClass(this, bTranslateDialogMessages, nContext, VERIFY(LoadAccelerators(hInstance, MAKEINTRESOURCE(nAcceleratorsResourceID)))).ParasitizeCurrentThread();
	} else {
		return xThreadParasiteClass(this, bTranslateDialogMessages, nContext, NULL).ParasitizeCurrentThread();
	}
}
std::intptr_t StandaloneWindowClass::GetMessageLoopContext() const {
	if(xm_pThreadParasite == nullptr){
		return 0;
	}
	return xm_pThreadParasite->GetContext();
}
