// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TAB_CONTROL_CLASS_HPP__
#define __MCF_TAB_CONTROL_CLASS_HPP__

#include "StdMCF.hpp"
#include <uxtheme.h>
#include <vector>
#include "WindowBaseClass.hpp"
#include "ModelessDialogClass.hpp"

namespace MCF {
	class TabDialogBaseClass : public ModelessDialogClass {
	public:
		TabDialogBaseClass(
			HINSTANCE			hInstance,
			int					nTemplateID,
			const DIPLITEM		*pDIPLItems = nullptr,
			std::size_t			uDIPLItemCount = 0
		) : ModelessDialogClass(hInstance, nTemplateID, pDIPLItems, uDIPLItemCount, nullptr, 0, 0) { }
		virtual ~TabDialogBaseClass(){ }
	public:
		void Create(HWND hParent){
			ModelessDialogClass::Create(hParent);

			EnableThemeDialogTexture(GetHWnd(), ETDT_ENABLETAB);
		}
		void Destroy(){
			ModelessDialogClass::Destroy();
		}
	};

	class TabControlClass final : public WindowBaseClass {
	private:
		class xParentSubclasserClass final : public WindowBaseClass {
		private:
			TabControlClass *const xm_pTabControl;
		public:
			xParentSubclasserClass(TabControlClass *pTabControl) : xm_pTabControl(pTabControl) { }
		protected:
			virtual bool xOnNotify(UINT uCode, UINT_PTR uSenderID, HWND hSender, const NMHDR *pNMHdr){
				if((hSender == xm_pTabControl->GetHWnd()) && (uCode == TCN_SELCHANGE)){
					xm_pTabControl->SwitchToTab((std::size_t)TabCtrl_GetCurSel(hSender));
				}
				return WindowBaseClass::xOnNotify(uCode, uSenderID, hSender, pNMHdr);
			}
		};
	private:
		xParentSubclasserClass *xm_pParentSubclasser;

		std::vector<TabDialogBaseClass *> xm_vecpTabDialogs;
		HIMAGELIST xm_hIconList;
		std::size_t xm_uCurrentTabIndex;
	public:
		TabControlClass();
		~TabControlClass();
	protected:
		virtual void xPostSubclass();
		virtual void xPreUnsubclass();
	public:
		std::size_t AttachTab(TabDialogBaseClass *pTabDialog, LPCTSTR pszCaption, HICON hIcon);
		TabDialogBaseClass *DetachTab(std::size_t uIndex);

		std::size_t SwitchToTab(std::size_t uNewTabIndex);
		void UpdateCurrentTab();

		std::size_t GetCurrentTabIndex() const;
		std::size_t GetTabCount() const;

		TabDialogBaseClass *GetTabDialogByIndex(std::size_t uIndex) const;
		TabDialogBaseClass *GetCurrentTabDialog() const;
	};
}

#endif
