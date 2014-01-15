// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "PopupMenuCreatorClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
PopupMenuCreatorClass::PopupMenuCreatorClass(){
}
PopupMenuCreatorClass::~PopupMenuCreatorClass(){
	DeleteAllItems();
}

// 其他非静态成员函数。
void PopupMenuCreatorClass::xDoDeployMenu(HMENU hMenu) const {
	ASSERT(hMenu != NULL);

	std::size_t uDefaultItemIndex = (std::size_t)-1;

	for(auto iter = xm_vecMenuItems.cbegin(); iter != xm_vecMenuItems.cend(); ++iter){
		switch(iter->eType){
			case COMMAND:
				VERIFY(AppendMenu(
					hMenu,
					(UINT)(MF_STRING
						| (iter->ItemData.bIsEnabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED))
						| (iter->ItemData.bIsChecked ? MF_CHECKED : MF_UNCHECKED)
					),
					(UINT_PTR)iter->ItemData.nCommandID,
					(LPCTSTR)iter->ItemData.strItemText
				));
				break;
			case SEPERATOR:
				VERIFY(AppendMenu(
					hMenu,
					(UINT)MF_SEPARATOR,
					(UINT_PTR)-1,
					nullptr
				));
				break;
			case SUBMENU:
				{
					const HMENU hSubMenu = VERIFY(CreatePopupMenu());
					VERIFY(AppendMenu(
						hMenu,
						(UINT)(MF_POPUP
							| (iter->ItemData.bIsEnabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED))
							| (iter->ItemData.bIsChecked ? MF_CHECKED : MF_UNCHECKED)
						),
						(UINT_PTR)hSubMenu,
						(LPCTSTR)iter->ItemData.strItemText
					));
					iter->ItemData.pSubMenu->xDoDeployMenu(hSubMenu);

					break;
				}
			default:
				ASSERT(FALSE);
		}
		if(iter->bIsDefaultItem){
			uDefaultItemIndex = (std::size_t)(iter - xm_vecMenuItems.begin());
		}
	}
	if(uDefaultItemIndex != (std::size_t)-1){
		::SetMenuDefaultItem(hMenu, uDefaultItemIndex, TRUE);
	}
}

void PopupMenuCreatorClass::AppendCommand(LPCTSTR pszItemText, int nCommandID, bool bIsEnabled, bool bIsChecked, bool bIsDefaultItem){
	xMENUITEMSTRUCT MenuItemStruct;

	MenuItemStruct.eType					= COMMAND;
	MenuItemStruct.ItemData.strItemText		= pszItemText;
	MenuItemStruct.ItemData.nCommandID		= nCommandID;
	MenuItemStruct.ItemData.bIsEnabled		= bIsEnabled;
	MenuItemStruct.ItemData.bIsChecked		= bIsChecked;
	MenuItemStruct.bIsDefaultItem			= bIsDefaultItem;

	xm_vecMenuItems.emplace_back(MenuItemStruct);
}
void PopupMenuCreatorClass::AppendSeperator(){
	xMENUITEMSTRUCT MenuItemStruct;

	// 对于分隔符来说，ItemData 是没有意义的。
	MenuItemStruct.eType					= SEPERATOR;
	MenuItemStruct.bIsDefaultItem			= false;

	xm_vecMenuItems.emplace_back(MenuItemStruct);
}
PopupMenuCreatorClass *PopupMenuCreatorClass::AppendSubMenu(LPCTSTR pszItemText, bool bIsEnabled, bool bIsChecked, bool bIsDefaultItem){
	xMENUITEMSTRUCT MenuItemStruct;

	MenuItemStruct.eType					= SUBMENU;
	MenuItemStruct.ItemData.strItemText		= pszItemText;
	MenuItemStruct.ItemData.pSubMenu		= new PopupMenuCreatorClass;
	MenuItemStruct.ItemData.bIsEnabled		= bIsEnabled;
	MenuItemStruct.ItemData.bIsChecked		= bIsChecked;
	MenuItemStruct.bIsDefaultItem			= bIsDefaultItem;

	xm_vecMenuItems.emplace_back(MenuItemStruct);

	return MenuItemStruct.ItemData.pSubMenu;
}
void PopupMenuCreatorClass::DeleteItem(std::size_t uIndex){
	const auto iter = xm_vecMenuItems.begin() + (int)uIndex;

	if(iter->eType == SUBMENU){
		delete iter->ItemData.pSubMenu;
	}
	xm_vecMenuItems.erase(iter);
}
void PopupMenuCreatorClass::DeleteAllItems(){
	while(!xm_vecMenuItems.empty()){
		DeleteItem(xm_vecMenuItems.size() - 1);
	}
}

void PopupMenuCreatorClass::TrackMenu(HWND hParent, long lXPos, long lYPos, const RECT *prectExcluded) const {
	const HMENU hPopupMenu = VERIFY(CreatePopupMenu());
	xDoDeployMenu(hPopupMenu);

	::SetForegroundWindow(hParent);
	if(prectExcluded == nullptr){
		::TrackPopupMenuEx(hPopupMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_TOPALIGN, lXPos, lYPos, hParent, nullptr);
	} else {
		::TPMPARAMS TPMParams;
		TPMParams.cbSize	= sizeof(TPMParams);
		TPMParams.rcExclude	= *prectExcluded;

		::TrackPopupMenuEx(hPopupMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_TOPALIGN, lXPos, lYPos, hParent, &TPMParams);
	}
	::PostMessage(hParent, WM_NULL, 0, 0);
	::DestroyMenu(hPopupMenu);
}
