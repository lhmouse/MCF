// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "OpenSaveDialogClass.hpp"
using namespace MCF;

// 静态成员变量。
TLSManagerTemplate<OpenSaveDialogClass *> OpenSaveDialogClass::xs_tlsCurrentInstance;

// 静态成员函数。
UINT_PTR CALLBACK OpenSaveDialogClass::xOFNHookProc(HWND hChild, UINT uMsg, WPARAM wParam, LPARAM lParam){
	const HWND hDialog = ::GetAncestor(hChild, GA_PARENT);
	OpenSaveDialogClass *pCurrentInstance = dynamic_cast<OpenSaveDialogClass *>(ModalDialogClass::GetWindowPtr(hDialog));
	if(pCurrentInstance == nullptr){
		OpenSaveDialogClass **const ppCurrentInstance = xs_tlsCurrentInstance;
		pCurrentInstance = *ppCurrentInstance;
		*ppCurrentInstance = nullptr;
		ASSERT(pCurrentInstance != nullptr);

		pCurrentInstance->Subclass(hDialog);
		if(pCurrentInstance->xm_pChildDialog != nullptr){
			pCurrentInstance->xm_pChildDialog->Subclass(hChild);
		}
	}

	switch(uMsg){
		case WM_INITDIALOG:
			if(pCurrentInstance->xOnOFNInitDialog((OPENFILENAME *)lParam)){
				::SetWindowLongPtr(hChild, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
			break;
		case WM_NOTIFY:
			if(pCurrentInstance->xOnOFNNotify(((LPNMHDR)lParam)->code, wParam, ((LPNMHDR)lParam)->hwndFrom, (LPNMHDR)lParam)){
				::SetWindowLongPtr(hChild, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
			break;
	}
	return FALSE;
}

// 构造函数和析构函数。
OpenSaveDialogClass::OpenSaveDialogClass(HWND hParent, LPCTSTR pszCaption, ModelessDialogClass *pChildDialog) :
	ModalDialogClass((pChildDialog == nullptr) ? NULL : pChildDialog->GetHInstance(), 0),

	xm_pChildDialog(pChildDialog)
{
	xm_strCaption = pszCaption;

	xm_OFNParams.lStructSize		= sizeof(xm_OFNParams);
	xm_OFNParams.hwndOwner			= hParent;
	xm_OFNParams.hInstance			= (pChildDialog == nullptr) ? NULL : pChildDialog->GetHInstance();
	xm_OFNParams.lpstrFilter		= nullptr;
	xm_OFNParams.lpstrCustomFilter	= nullptr;
	xm_OFNParams.nMaxCustFilter		= 0;
	xm_OFNParams.nFilterIndex		= 0;
	xm_OFNParams.lpstrFile			= nullptr;
	xm_OFNParams.nMaxFile			= 0;
	xm_OFNParams.lpstrFileTitle		= nullptr;
	xm_OFNParams.nMaxFileTitle		= 0;
	xm_OFNParams.lpstrInitialDir	= nullptr;
	xm_OFNParams.lpstrTitle			= (pszCaption == nullptr) ? nullptr : (LPCTSTR)xm_strCaption;
	xm_OFNParams.Flags				= 0;
	xm_OFNParams.nFileOffset		= 0;
	xm_OFNParams.nFileExtension		= 0;
	xm_OFNParams.lpstrDefExt		= nullptr;
	xm_OFNParams.lCustData			= 0;
	xm_OFNParams.lpfnHook			= &xOFNHookProc;
	xm_OFNParams.lpTemplateName		= (pChildDialog == nullptr) ? nullptr : MAKEINTRESOURCE(pChildDialog->GetTemplateID());
	xm_OFNParams.pvReserved			= nullptr;
	xm_OFNParams.dwReserved			= 0;
	xm_OFNParams.FlagsEx			= 0;

	xResizeFilePathBuffer(255);
}
OpenSaveDialogClass::~OpenSaveDialogClass(){
}

// 其他非静态成员函数。
void OpenSaveDialogClass::xResizeFilePathBuffer(std::size_t uNewLength){
	xm_strFilePathBuffer.Resize(uNewLength);
	xm_OFNParams.lpstrFile = xm_strFilePathBuffer;
	xm_OFNParams.nMaxFile = xm_strFilePathBuffer.GetCapacity() + 1;
}
void OpenSaveDialogClass::xUpdateDefExt(){
	ASSERT(GetHWnd() != NULL);

	if(xm_OFNParams.nFilterIndex > 0){
		const LPCTSTR pszCurrentFilter = xm_strDefExts.data() + (std::size_t)(xm_OFNParams.nFilterIndex - 1) * 4;
		if(pszCurrentFilter[0] == 0){
			SendMessage(CDM_SETDEFEXT, 0, (LPARAM)nullptr);
		} else {
			SendMessage(CDM_SETDEFEXT, 0, (LPARAM)pszCurrentFilter);
		}
	}
}

bool OpenSaveDialogClass::xOnCommand(int nControlID, WORD wNotifyCode, HWND hSender){
	if((wNotifyCode == CBN_EDITCHANGE) && (nControlID == cmb13)){
		xResizeFilePathBuffer((std::size_t)SendMessage(CDM_GETFOLDERPATH, 0, 0) + (std::size_t)SendMessage(CDM_GETSPEC, 0, 0));
	}
	return WindowBaseClass::xOnCommand(nControlID, wNotifyCode, hSender);
}

bool OpenSaveDialogClass::xOnOFNInitDialog(OPENFILENAME *pOFNParams){
	UNREF_PARAM(pOFNParams);

	xUpdateDefExt();

	return false;
}
bool OpenSaveDialogClass::xOnOFNNotify(UINT uCode, UINT_PTR uSenderID, HWND hSender, const NMHDR *pNMHdr){
	UNREF_PARAM(uSenderID);
	UNREF_PARAM(hSender);
	UNREF_PARAM(pNMHdr);

	switch(uCode){
		case CDN_SELCHANGE:
			xResizeFilePathBuffer((std::size_t)SendMessage(CDM_GETFOLDERPATH, 0, 0) + (std::size_t)SendMessage(CDM_GETSPEC, 0, 0));
			break;
		case CDN_TYPECHANGE:
			xUpdateDefExt();
			break;
	}

	return false;
}

void OpenSaveDialogClass::SetFilters(const OpenSaveDialogClass::FILTER *pFilters, std::size_t uFilterCount){
	xm_strFilters.clear();
	xm_strDefExts.clear();
	xm_OFNParams.nFilterIndex = 0;

	if(pFilters != nullptr){
		for(std::size_t i = 0; i < uFilterCount; ++i){
			xm_strFilters.append(pFilters[i].pszDescription, _tcslen(pFilters[i].pszDescription) + 1);
			xm_strFilters.append(pFilters[i].pszFilter, _tcslen(pFilters[i].pszFilter) + 1);

			xm_strDefExts.append(pFilters[i].achDefExt, 4);

			if(pFilters[i].bIsDefault){
				xm_OFNParams.nFilterIndex = (DWORD)(i + 1);
			}
		}
	}
	xm_strFilters.push_back(0);

	xm_OFNParams.lpstrFilter = xm_strFilters.data();
}
void OpenSaveDialogClass::SetInitialDir(LPCTSTR pszInitialDir){
	xm_strInitialDir = pszInitialDir;
	xm_OFNParams.lpstrInitialDir = (pszInitialDir == nullptr) ? nullptr : xm_strInitialDir;
}
void OpenSaveDialogClass::SetInitialFileName(LPCTSTR pszInitialFileName){
	if(pszInitialFileName == nullptr){
		xm_OFNParams.lpstrFile[0] = 0;
	} else {
		xm_strFilePathBuffer = pszInitialFileName;
		xm_OFNParams.lpstrFile = xm_strFilePathBuffer;
		xm_OFNParams.nMaxFile = xm_strFilePathBuffer.GetCapacity();
	}
}

bool OpenSaveDialogClass::ShowOpen(bool bEnableSizing, bool bAllowMultiSelect, bool bFileMustExist, bool bShowReadOnly, bool bInitialReadOnly){
	xm_OFNParams.Flags = (DWORD)(OFN_PATHMUSTEXIST | OFN_ENABLEHOOK | OFN_EXPLORER |
		((xm_pChildDialog != nullptr)	? OFN_ENABLETEMPLATE	: 0) |
		(bEnableSizing					? OFN_ENABLESIZING		: 0) |
		(bAllowMultiSelect				? OFN_ALLOWMULTISELECT	: 0) |
		(bFileMustExist					? OFN_FILEMUSTEXIST		: OFN_CREATEPROMPT) |
		(!bShowReadOnly					? OFN_HIDEREADONLY		: 0) |
		(bInitialReadOnly				? OFN_READONLY			: 0)
	);

	OpenSaveDialogClass **const ppCurrentInstance = xs_tlsCurrentInstance;
	ASSERT(*ppCurrentInstance == nullptr);
	*ppCurrentInstance = this;

	if(::GetOpenFileName(&xm_OFNParams) == FALSE){
		xm_OFNParams.lpstrFile[0] = 0;
		return false;
	}

	if(xm_OFNParams.nFileOffset > 0){
		// 截断目录和文件名。
		xm_OFNParams.lpstrFile[(std::size_t)xm_OFNParams.nFileOffset - 1] = 0;

		const LPCTSTR pszFileName = xm_OFNParams.lpstrFile + (std::ptrdiff_t)xm_OFNParams.nFileOffset;
		const std::size_t uFileNameLen = _tcslen(pszFileName);
		// 以两个 0 截断路径。
		xm_OFNParams.lpstrFile[xm_OFNParams.nFileOffset + uFileNameLen + 1] = 0;
	}
	return true;
}
bool OpenSaveDialogClass::ShowSave(bool bEnableSizing, bool bPromptBeforeOverwrite){
	xm_OFNParams.Flags = (DWORD)(OFN_PATHMUSTEXIST | OFN_ENABLEHOOK | OFN_EXPLORER |
		((xm_pChildDialog != nullptr)	? OFN_ENABLETEMPLATE	: 0) |
		(bEnableSizing					? OFN_ENABLESIZING		: 0) |
		(bPromptBeforeOverwrite			? OFN_OVERWRITEPROMPT	: 0)
	);

	OpenSaveDialogClass **const ppCurrentInstance = xs_tlsCurrentInstance;
	ASSERT(*ppCurrentInstance == nullptr);
	*ppCurrentInstance = this;

	if(::GetSaveFileName(&xm_OFNParams) != FALSE){
		xm_OFNParams.lpstrFile[0] = 0;
		return false;
	}

	// 我们并不打算让保存对话框可以多选。
	return true;
}

std::vector<LPCTSTR> OpenSaveDialogClass::GetPathAndNames() const {
	std::vector<LPCTSTR> vecRet;
	for(LPCTSTR pszCur = xm_OFNParams.lpstrFile; pszCur[0] != 0; pszCur += _tcslen(pszCur) + 1){
		vecRet.emplace_back(pszCur);
	}
	return std::move(vecRet);
}
bool OpenSaveDialogClass::WasReadOnlyChecked() const {
	return (xm_OFNParams.Flags & OFN_READONLY) != 0;
}
