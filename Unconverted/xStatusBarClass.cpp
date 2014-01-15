// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "xStatusBarClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
StandaloneWindowClass::xStatusBarClass::xStatusBarClass(StandaloneWindowClass *pOwner) :
	xm_pOwner		(pOwner)
{
	ASSERT(xm_pOwner != nullptr);

	Subclass(VERIFY(::CreateWindowEx(0, STATUSCLASSNAME, nullptr, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, 0, 0, 0, xm_pOwner->GetHWnd(), (HMENU)-1, ::GetModuleHandle(nullptr), nullptr)));
	SendMessage(SB_SETMINHEIGHT, (WPARAM)xm_pOwner->xm_uStatusBarMinHeight);
}
StandaloneWindowClass::xStatusBarClass::~xStatusBarClass(){
	::DestroyWindow(GetHWnd());
}

// 其他非静态成员函数。
void StandaloneWindowClass::xStatusBarClass::Update(){
	RECT rectClient = xm_pOwner->GetClientRect();
	const std::size_t uPartCount = xm_pOwner->xm_vecuStatusBarWidths.size();
	std::vector<std::size_t> vecuCurrentWidths(uPartCount);
	std::size_t uAutoPartIndex = (std::size_t)-1;

	for(std::size_t i = 0; i < uPartCount; ++i){
		if(xm_pOwner->xm_vecuStatusBarWidths[i] == (std::size_t)-1){
			ASSERT(uAutoPartIndex == (std::size_t)-1);

			uAutoPartIndex = i;
		} else {
			rectClient.right -= xm_pOwner->xm_vecuStatusBarWidths[i];
			vecuCurrentWidths[i] = xm_pOwner->xm_vecuStatusBarWidths[i];
		}
	}
	if(uAutoPartIndex != -1){
		vecuCurrentWidths[uAutoPartIndex] = (std::size_t)rectClient.right;
	}
	for(std::size_t i = 1; i < uPartCount; ++i){
		vecuCurrentWidths[i] += vecuCurrentWidths[i - 1];
	}

	SendMessage(SB_SETPARTS, (WPARAM)uPartCount, (LPARAM)vecuCurrentWidths.data());
	SendMessage(WM_SIZE);
}
