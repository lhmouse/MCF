// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_NOTIFY_ICON_MANAGER_CLASS_HPP__
#define __MCF_NOTIFY_ICON_MANAGER_CLASS_HPP__

#include "StdMCF.hpp"
#include <ShellAPI.h>

namespace MCF {
	class NotifyIconManagerClass final : NO_COPY_OR_ASSIGN {
	private:
		static std::atomic<std::size_t> xs_uIconCounter;
	private:
		const std::size_t xm_uIconID;
		const HWND xm_hOwner;
		const unsigned int xm_uMessageID;
	public:
		NotifyIconManagerClass(HWND hOwner, UINT uMessageID, HICON hInitialIcon, LPCTSTR pszInitialTip, bool bInitiallyVisible = true);
		~NotifyIconManagerClass();
	public:
		void Hide();
		void Show();
		void SetIcon(HICON hNewIcon);
		void SetTip(LPCTSTR pszNewTip);
		void PopBalloon(LPCTSTR pszInfo, LPCTSTR pszTitle = nullptr, UINT uTimeout = 1500, DWORD dwFlags = NIIF_NONE, HICON hIcon = NULL);
	};
}

#endif
