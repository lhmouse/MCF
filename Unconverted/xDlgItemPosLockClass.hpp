// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_DLG_ITEM_POS_LOCK_CLASS_HPP__
#define __MCF_X_DLG_ITEM_POS_LOCK_CLASS_HPP__

#include "StdMCF.hpp"
#include <vector>
#include "DialogBaseClass.hpp"

namespace MCF {
	// 控件位置锁，锁定控件的指定边界与父窗体边界以一定的方式锁定（相对坐标固定或成比例变化）。
	class DialogBaseClass::xDlgItemPosLockClass final : NO_COPY_OR_ASSIGN {
	private:
		typedef struct xtagSavedCoords {
			HWND hCtrl;
			RECT rectDelta;
		} xSAVEDCOORDS;
	private:
		DialogBaseClass *const xm_pOwner;
		const std::vector<DialogBaseClass::DIPLITEM> xm_vecDIPLItems;

		std::vector<xSAVEDCOORDS> xm_vecSavedCoords;
	public:
		xDlgItemPosLockClass(DialogBaseClass *pOwner, const DialogBaseClass::DIPLITEM *pDIPLItems, std::size_t uItemCount);
		~xDlgItemPosLockClass();
	public:
		void Update();
	};
}

#endif
