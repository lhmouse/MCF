// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_OPEN_SAVE_DIALOG_CLASS_HPP__
#define __MCF_OPEN_SAVE_DIALOG_CLASS_HPP__

#include "StdMCF.hpp"
#include <string>
#include <vector>
#include "Memory.hpp"
#include "ModalDialogClass.hpp"
#include "ModelessDialogClass.hpp"

namespace MCF {
	class OpenSaveDialogClass : public ModalDialogClass {
	public:
		typedef struct xtagFilter {
			LPCTSTR pszDescription;
			LPCTSTR pszFilter;
			TCHAR achDefExt[4];		// 必须以 0 结尾。
			bool bIsDefault;
		} FILTER;
	private:
		static TLSManagerTemplate<OpenSaveDialogClass *> xs_tlsCurrentInstance;
	private:
		static UINT_PTR CALLBACK xOFNHookProc(HWND hChild, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		ModelessDialogClass *const xm_pChildDialog;

		TString xm_strCaption;
		TString xm_strInitialDir;
		TString xm_strFilePathBuffer;
		std::basic_string<TCHAR> xm_strFilters;
		std::basic_string<TCHAR> xm_strDefExts;		// 以四个字符为单位。

		OPENFILENAME xm_OFNParams;
	public:
		OpenSaveDialogClass(HWND hParent, LPCTSTR pszCaption, ModelessDialogClass *pChildDialog = nullptr);
		virtual ~OpenSaveDialogClass();
	private:
		void xResizeFilePathBuffer(std::size_t uNewLength);
		void xUpdateDefExt();
	protected:
		virtual bool xOnCommand(int nControlID, WORD wNotifyCode, HWND hSender);

		virtual bool xOnOFNInitDialog(OPENFILENAME *pOFNParams);
		virtual bool xOnOFNNotify(UINT uCode, UINT_PTR uSenderID, HWND hSender, const NMHDR *pNMHdr);
	public:
		void SetFilters(const FILTER *pFilters, std::size_t uFilterCount);
		void SetInitialDir(LPCTSTR pszInitialDir);
		void SetInitialFileName(LPCTSTR pszInitialFileName);

		bool ShowOpen(bool bEnableSizing = false, bool bAllowMultiSelect = false, bool bFileMustExist = true, bool bShowReadOnly = false, bool bInitialReadOnly = false);
		bool ShowSave(bool bEnableSizing = false, bool bPromptBeforeOverwrite = true);

		// 向量中第一个元素是目录的路径，第二个起是文件名。如果打开失败返回一个空的向量。
		std::vector<LPCTSTR> GetPathAndNames() const;
		bool WasReadOnlyChecked() const;
	};
}

#endif
