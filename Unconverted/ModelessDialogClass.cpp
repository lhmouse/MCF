// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "ModelessDialogClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
ModelessDialogClass::ModelessDialogClass(
	HINSTANCE			hInstance,
	int					nTemplateID,
	const DIPLITEM		*pDIPLItems,
	std::size_t			uDIPLItemCount,
	const std::size_t	*puStatusBarPartWidths,
	std::size_t			uStatusBarPartCount,
	std::size_t			uStatusBarMinHeight
) :
	DialogBaseClass(hInstance, nTemplateID, pDIPLItems, uDIPLItemCount, puStatusBarPartWidths, uStatusBarPartCount, uStatusBarMinHeight)
{
}
ModelessDialogClass::~ModelessDialogClass(){
}

// 其他非静态成员函数。
void ModelessDialogClass::Create(HWND hParent){
	ASSERT(GetHWnd() == NULL);

	DialogBaseClass **const ppCurrentInstance = DialogBaseClass::xs_tlsCurrentInstance;
	ASSERT(*ppCurrentInstance == nullptr);
	*ppCurrentInstance = this;

	const HWND hCreatedDialog = VERIFY(CreateDialogParam(GetHInstance(), MAKEINTRESOURCE(GetTemplateID()), hParent, &xDlgProcStub, 0));

	UNREF_PARAM(hCreatedDialog);

	ASSERT(GetHWnd() == hCreatedDialog);
}
void ModelessDialogClass::Destroy(){
	const HWND hDlg = GetHWnd();

	if(hDlg != NULL){
		::DestroyWindow(hDlg);

		ASSERT(GetHWnd() == NULL);
	}
}
