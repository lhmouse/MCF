// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "NotifyIconManagerClass.hpp"
using namespace MCF;

// 静态成员变量定义。
std::atomic<std::size_t> NotifyIconManagerClass::xs_uIconCounter(0);

// 构造函数和析构函数。
// 调用者必须负责 hInitialIcon 的创建和销毁。
NotifyIconManagerClass::NotifyIconManagerClass(HWND hOwner, UINT uMessageID, HICON hInitialIcon, LPCTSTR pszInitialTip, bool bInitiallyVisible) :
	xm_uIconID		(++xs_uIconCounter),
	xm_hOwner		(hOwner),
	xm_uMessageID	(uMessageID)
{
	ASSERT(hOwner != NULL);

	// 在通知区上添加图标。
	NOTIFYICONDATA nidataNotifyIconData;
#if(NTDDI_VERSION >= NTDDI_VISTA)
	nidataNotifyIconData.cbSize				= NOTIFYICONDATA_V3_SIZE;
#else
	nidataNotifyIconData.cbSize				= sizeof(nidataNotifyIconData);
#endif
	nidataNotifyIconData.hWnd				= xm_hOwner;
	nidataNotifyIconData.uID				= xm_uIconID;
	nidataNotifyIconData.uFlags				= NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_STATE;
	nidataNotifyIconData.uCallbackMessage	= xm_uMessageID;
	nidataNotifyIconData.hIcon				= hInitialIcon;
	_tcscpy_s(nidataNotifyIconData.szTip, pszInitialTip);
	nidataNotifyIconData.uVersion			= NOTIFYICON_VERSION;
	nidataNotifyIconData.dwState			= bInitiallyVisible ? 0 : (DWORD)NIS_HIDDEN;
	nidataNotifyIconData.dwStateMask		= NIS_HIDDEN;
	Shell_NotifyIcon(NIM_ADD, &nidataNotifyIconData);
	Shell_NotifyIcon(NIM_SETVERSION, &nidataNotifyIconData);
}
NotifyIconManagerClass::~NotifyIconManagerClass(){
	// 删除通知区的图标。
	NOTIFYICONDATA nidataNotifyIconData;
#if(NTDDI_VERSION >= NTDDI_VISTA)
	nidataNotifyIconData.cbSize				= NOTIFYICONDATA_V3_SIZE;
#else
	nidataNotifyIconData.cbSize				= sizeof(nidataNotifyIconData);
#endif
	nidataNotifyIconData.hWnd				= xm_hOwner;
	nidataNotifyIconData.uID				= xm_uIconID;
	Shell_NotifyIcon(NIM_DELETE, &nidataNotifyIconData);
}

// 其他非静态成员函数。
void NotifyIconManagerClass::Hide(){
	NOTIFYICONDATA nidataNotifyIconData;
#if(NTDDI_VERSION >= NTDDI_VISTA)
	nidataNotifyIconData.cbSize				= NOTIFYICONDATA_V3_SIZE;
#else
	nidataNotifyIconData.cbSize				= sizeof(nidataNotifyIconData);
#endif
	nidataNotifyIconData.hWnd				= xm_hOwner;
	nidataNotifyIconData.uID				= xm_uIconID;
	nidataNotifyIconData.uFlags				= NIF_STATE;
	nidataNotifyIconData.dwState			= NIS_HIDDEN;
	nidataNotifyIconData.dwStateMask		= NIS_HIDDEN;
	Shell_NotifyIcon(NIM_MODIFY, &nidataNotifyIconData);
}
void NotifyIconManagerClass::Show(){
	NOTIFYICONDATA nidataNotifyIconData;
#if(NTDDI_VERSION >= NTDDI_VISTA)
	nidataNotifyIconData.cbSize				= NOTIFYICONDATA_V3_SIZE;
#else
	nidataNotifyIconData.cbSize				= sizeof(nidataNotifyIconData);
#endif
	nidataNotifyIconData.hWnd				= xm_hOwner;
	nidataNotifyIconData.uID				= xm_uIconID;
	nidataNotifyIconData.uFlags				= NIF_STATE;
	nidataNotifyIconData.dwState			= 0;
	nidataNotifyIconData.dwStateMask		= NIS_HIDDEN;
	Shell_NotifyIcon(NIM_MODIFY, &nidataNotifyIconData);
}
// 调用者必须负责 hNewIcon 的创建和销毁。
void NotifyIconManagerClass::SetIcon(HICON hNewIcon){
	NOTIFYICONDATA nidataNotifyIconData;
#if(NTDDI_VERSION >= NTDDI_VISTA)
	nidataNotifyIconData.cbSize				= NOTIFYICONDATA_V3_SIZE;
#else
	nidataNotifyIconData.cbSize				= sizeof(nidataNotifyIconData);
#endif
	nidataNotifyIconData.hWnd				= xm_hOwner;
	nidataNotifyIconData.uID				= xm_uIconID;
	nidataNotifyIconData.uFlags				= NIF_ICON;
	nidataNotifyIconData.hIcon				= hNewIcon;
	Shell_NotifyIcon(NIM_MODIFY, &nidataNotifyIconData);
}
// 更改鼠标悬停时显示的工具提示。
// 注意不是气球提示。
void NotifyIconManagerClass::SetTip(LPCTSTR pszNewTip){
	NOTIFYICONDATA nidataNotifyIconData;
#if(NTDDI_VERSION >= NTDDI_VISTA)
	nidataNotifyIconData.cbSize				= NOTIFYICONDATA_V3_SIZE;
#else
	nidataNotifyIconData.cbSize				= sizeof(nidataNotifyIconData);
#endif
	nidataNotifyIconData.hWnd				= xm_hOwner;
	nidataNotifyIconData.uID				= xm_uIconID;
	nidataNotifyIconData.uFlags				= NIF_TIP;
	if(pszNewTip == nullptr){
		nidataNotifyIconData.szTip[0]		= 0;
	} else {
		_tcscpy_s(nidataNotifyIconData.szTip, pszNewTip);
	}
	Shell_NotifyIcon(NIM_MODIFY, &nidataNotifyIconData);
}
// 显示气球提示。注意不是工具提示。
// 气球提示有一个标题，一段内容，显示延迟以及图标。
// dwFlags		NIIF_NONE		不显示图标；
//				NIIF_ERROR		显示红色 X 图标；
//				NIIF_INFO		显示蓝色 i 图标；
//				NIIF_WARNING	显示黄色 ! 图标；
//				NIIF_USER		显示 hIcon 指定的图标。
// 调用者必须负责 hIcon 的创建和销毁。
void NotifyIconManagerClass::PopBalloon(LPCTSTR pszInfo, LPCTSTR pszTitle, UINT uTimeout, DWORD dwFlags, HICON hIcon){
	NOTIFYICONDATA nidataNotifyIconData;
#if(NTDDI_VERSION >= NTDDI_VISTA)
	nidataNotifyIconData.cbSize				= NOTIFYICONDATA_V3_SIZE;
#else
	nidataNotifyIconData.cbSize				= sizeof(nidataNotifyIconData);
#endif
	nidataNotifyIconData.hWnd				= xm_hOwner;
	nidataNotifyIconData.uID				= xm_uIconID;
	nidataNotifyIconData.uFlags				= NIF_INFO;
	if(pszInfo == nullptr){
		nidataNotifyIconData.szInfo[0] = 0;
	} else {
		_tcscpy_s(nidataNotifyIconData.szInfo, pszInfo);
	}
	if(pszTitle == nullptr){
		nidataNotifyIconData.szInfoTitle[0] = 0;
	} else {
		_tcscpy_s(nidataNotifyIconData.szInfoTitle, pszTitle);
	}
	nidataNotifyIconData.uTimeout			= uTimeout;
	nidataNotifyIconData.dwInfoFlags		= dwFlags;
	nidataNotifyIconData.hIcon				= hIcon;
	Shell_NotifyIcon(NIM_MODIFY, &nidataNotifyIconData);
}
