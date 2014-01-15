// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "TabControlClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
TabControlClass::TabControlClass(){
	xm_uCurrentTabIndex = (std::size_t)-1;

	xm_pParentSubclasser = new xParentSubclasserClass(this);
}
TabControlClass::~TabControlClass(){
	delete xm_pParentSubclasser;
}

// 其他非静态成员函数。
void TabControlClass::xPostSubclass(){
	WindowBaseClass::xPostSubclass();

	WindowBaseClass *const pParent = GetParent();
	if(pParent != nullptr){
		xm_pParentSubclasser->SubclassAfter(*pParent);
	}
	xm_uCurrentTabIndex = (std::size_t)-1;
	xm_hIconList = VERIFY(::ImageList_Create(16, 16, ILC_COLOR32, 0, 1));
	SendMessage(TCM_SETIMAGELIST, 0, (LPARAM)xm_hIconList);
	SetStyle(WS_CLIPSIBLINGS, 0xFFFFFFFF);
}
void TabControlClass::xPreUnsubclass(){
	xm_uCurrentTabIndex = (std::size_t)-1;
	while(xm_vecpTabDialogs.size() != 0){
		DetachTab(0);
	}
	SendMessage(TCM_SETIMAGELIST, 0, (LPARAM)NULL);
	::ImageList_Destroy(xm_hIconList);

	WindowBaseClass::xPreUnsubclass();
}

std::size_t TabControlClass::AttachTab(TabDialogBaseClass *pTabDialog, LPCTSTR pszCaption, HICON hIcon){
	ASSERT(GetHWnd() != NULL);

	pTabDialog->Show(SW_HIDE);

	// 设置图标和文本。
	TCITEM tciCurrentItem;
	tciCurrentItem.mask = TCIF_TEXT | TCIF_IMAGE;
	tciCurrentItem.pszText = (LPTSTR)pszCaption;
	if(hIcon == NULL){
		tciCurrentItem.iImage = -1;
	} else {
		tciCurrentItem.iImage = ImageList_AddIcon(xm_hIconList, hIcon);
	}
	tciCurrentItem.lParam = 0;
	SendMessage(TCM_INSERTITEM, xm_vecpTabDialogs.size(), (LPARAM)&tciCurrentItem);

	xm_vecpTabDialogs.emplace_back(pTabDialog);
	return xm_vecpTabDialogs.size() - 1;
}
TabDialogBaseClass *TabControlClass::DetachTab(std::size_t uIndex){
	ASSERT(GetHWnd() != NULL);

	const std::size_t uOldTabIndex = xm_uCurrentTabIndex;

	SwitchToTab((std::size_t)-1);
	SendMessage(TCM_DELETEITEM, uIndex);

	const auto iter = xm_vecpTabDialogs.cbegin() + (int)uIndex;
	TabDialogBaseClass *const pDialogRet = *iter;
	xm_vecpTabDialogs.erase(iter);

	if(uOldTabIndex != -1){
		const std::size_t uTabsRemaining = xm_vecpTabDialogs.size();
		if(uTabsRemaining == 0){
			SwitchToTab((std::size_t)-1);
		} else if(uOldTabIndex >= xm_vecpTabDialogs.size()){
			SwitchToTab(uOldTabIndex - 1);
		} else {
			SwitchToTab(uOldTabIndex);
		}
	}

	return pDialogRet;
}

std::size_t TabControlClass::SwitchToTab(std::size_t uNewTabIndex){
	const std::size_t uOldTabIndex = xm_uCurrentTabIndex;

	if(uNewTabIndex != xm_uCurrentTabIndex){
		if(xm_uCurrentTabIndex != (std::size_t)-1){
			xm_vecpTabDialogs.at(xm_uCurrentTabIndex)->Show(SW_HIDE);
		}
		if(uNewTabIndex != (std::size_t)-1){
			xm_vecpTabDialogs.at(uNewTabIndex)->Show(SW_SHOW);
		}
		SendMessage(TCM_SETCURSEL, uNewTabIndex);

		xm_uCurrentTabIndex = uNewTabIndex;
		UpdateCurrentTab();
	}

	return uOldTabIndex;
}
void TabControlClass::UpdateCurrentTab(){
	if(xm_uCurrentTabIndex != -1){
		const HWND hCurrentTab = xm_vecpTabDialogs.at(xm_uCurrentTabIndex)->GetHWnd();

		RECT rectTab = GetRect();
		SendMessage(TCM_ADJUSTRECT, FALSE, (LPARAM)&rectTab);

		SetWindowPos(hCurrentTab, HWND_TOP, rectTab.left, rectTab.top, rectTab.right - rectTab.left, rectTab.bottom - rectTab.top, 0);
		Update();
	}
}

std::size_t TabControlClass::GetCurrentTabIndex() const {
	return xm_uCurrentTabIndex;
}
std::size_t TabControlClass::GetTabCount() const {
	return xm_vecpTabDialogs.size();
}

TabDialogBaseClass *TabControlClass::GetTabDialogByIndex(std::size_t uIndex) const {
	if(uIndex != -1){
		return xm_vecpTabDialogs.at(uIndex);
	}
	return nullptr;
}
TabDialogBaseClass *TabControlClass::GetCurrentTabDialog() const {
	return GetTabDialogByIndex(xm_uCurrentTabIndex);
}
