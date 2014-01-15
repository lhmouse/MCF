// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_POPUP_MENU_CREATOR_CLASS_HPP__
#define __MCF_POPUP_MENU_CREATOR_CLASS_HPP__

#include "StdMCF.hpp"
#include <vector>

namespace MCF {
	class PopupMenuCreatorClass final : NO_COPY_OR_ASSIGN {
	private:
		typedef enum {
			COMMAND,
			SEPERATOR,
			SUBMENU
		} xMENUTYPE;
		typedef struct xtagMenuItemStruct {
			xMENUTYPE eType;
			struct {
				TString strItemText;
				union {
					int nCommandID;
					PopupMenuCreatorClass *pSubMenu;
				};
				bool bIsEnabled;
				bool bIsChecked;
			} ItemData;
			bool bIsDefaultItem;
		} xMENUITEMSTRUCT;
	private:
		std::vector<xMENUITEMSTRUCT> xm_vecMenuItems;
	public:
		PopupMenuCreatorClass();
		~PopupMenuCreatorClass();
	private:
		void xDoDeployMenu(HMENU hMenu) const;
	public:
		void AppendCommand(LPCTSTR pszItemText, int nCommandID, bool bIsEnabled = true, bool bIsChecked = false, bool bIsDefaultItem = false);
		void AppendSeperator();
		PopupMenuCreatorClass *AppendSubMenu(LPCTSTR pszItemText, bool bIsEnabled = true, bool bIsChecked = false, bool bIsDefaultItem = false);
		void DeleteItem(std::size_t uIndex);
		void DeleteAllItems();

		void TrackMenu(HWND hParent, long lXPos, long lYPos, const RECT *prectExcluded = nullptr) const;
	};
}

#endif
