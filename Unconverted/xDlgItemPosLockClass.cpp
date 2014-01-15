// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "DialogBaseClass.hpp"
#include "xDlgItemPosLockClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
DialogBaseClass::xDlgItemPosLockClass::xDlgItemPosLockClass(DialogBaseClass *pOwner, const DialogBaseClass::DIPLITEM *pDIPLItems, std::size_t uItemCount) :
	xm_pOwner			(pOwner),
	xm_vecDIPLItems		(pDIPLItems, pDIPLItems + uItemCount),

	xm_vecSavedCoords	(uItemCount)
{
	ASSERT(xm_pOwner != nullptr);

	const RECT rectDialogClient = xm_pOwner->GetClientRect();

	for(std::size_t i = 0; i < xm_vecDIPLItems.size(); ++i){
		const HWND hCtrl = VERIFY(xm_pOwner->GetDlgItemHWnd(pDIPLItems[i].nCtrlID));

		xm_vecSavedCoords[i].hCtrl = hCtrl;
		const RECT rectControl = xm_pOwner->GetChildRect(hCtrl);

		xm_vecSavedCoords[i].rectDelta.left		= rectControl.left		- (pDIPLItems[i].bLeftEdgeLockedUponRight		? rectDialogClient.right	: 0);
		xm_vecSavedCoords[i].rectDelta.top		= rectControl.top		- (pDIPLItems[i].bTopEdgeLockedUponBottom		? rectDialogClient.bottom	: 0);
		xm_vecSavedCoords[i].rectDelta.right	= rectControl.right		- (pDIPLItems[i].bRightEdgeLockedUponRight		? rectDialogClient.right	: 0);
		xm_vecSavedCoords[i].rectDelta.bottom	= rectControl.bottom	- (pDIPLItems[i].bBottomEdgeLockedUponBottom	? rectDialogClient.bottom	: 0);
	}
}
DialogBaseClass::xDlgItemPosLockClass::~xDlgItemPosLockClass(){
}

// 根据控件位置锁和历史信息更新控件位置及大小。
void DialogBaseClass::xDlgItemPosLockClass::Update(){
	RECT rectDialogClient = xm_pOwner->GetClientRect();

	const HWND hStatusBar = xm_pOwner->GetStatusBarHWnd();
	if(hStatusBar != NULL){
		RECT rectStatusBar;
		::GetWindowRect(hStatusBar, &rectStatusBar);
		rectDialogClient.bottom -= rectStatusBar.bottom - rectStatusBar.top;
	}

	for(std::size_t i = 0; i < xm_vecDIPLItems.size(); ++i){
		const int nLeft		= xm_vecSavedCoords[i].rectDelta.left	+ (xm_vecDIPLItems[i].bLeftEdgeLockedUponRight		? rectDialogClient.right	: 0);
		const int nTop		= xm_vecSavedCoords[i].rectDelta.top	+ (xm_vecDIPLItems[i].bTopEdgeLockedUponBottom		? rectDialogClient.bottom	: 0);
		const int nRight	= xm_vecSavedCoords[i].rectDelta.right	+ (xm_vecDIPLItems[i].bRightEdgeLockedUponRight		? rectDialogClient.right	: 0);
		const int nBottom	= xm_vecSavedCoords[i].rectDelta.bottom	+ (xm_vecDIPLItems[i].bBottomEdgeLockedUponBottom	? rectDialogClient.bottom	: 0);

		::SetWindowPos(xm_vecSavedCoords[i].hCtrl, NULL, nLeft, nTop, nRight - nLeft, nBottom - nTop, SWP_NOZORDER);
	}
}
