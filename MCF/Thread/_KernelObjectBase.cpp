// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_KernelObjectBase.hpp"
#include "../../MCFCRT/ext/wcpcpy.h"
#include "../../MCFCRT/ext/itow.h"
#include "../Core/Exception.hpp"
#include <winternl.h>
#include <ntdef.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtOpenDirectoryObject(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) noexcept;

namespace MCF {

namespace Impl_KernelObjectBase {
	Impl_UniqueNtHandle::UniqueNtHandle KernelObjectBase::Y_OpenBaseNamedObjectDirectory(std::uint32_t u32Flags){
		static constexpr wchar_t kSessionPathPrefix[] = L"\\Sessions\\";
		static constexpr wchar_t kBaseNameObjects[]   = L"\\BaseNamedObjects";

		wchar_t awcNameBuffer[128];
		wchar_t *pwcBegin, *pwcEnd;
		if(u32Flags & kGlobal){
			pwcBegin = (wchar_t *)kBaseNameObjects;
			pwcEnd   = pwcBegin;
		} else {
			pwcBegin = awcNameBuffer;
			pwcEnd   = pwcBegin;

			pwcEnd = ::_MCFCRT_wcpcpy(pwcEnd, kSessionPathPrefix);
			pwcEnd = ::_MCFCRT_itow_u(pwcEnd, ::WTSGetActiveConsoleSessionId());
			pwcEnd = ::_MCFCRT_wcpcpy(pwcEnd, kBaseNameObjects);
		}

		::UNICODE_STRING ustrName;
		ustrName.Length        = (unsigned short)((char *)pwcEnd - (char *)pwcBegin);
		ustrName.MaximumLength = ustrName.Length;
		ustrName.Buffer        = pwcBegin;

		::OBJECT_ATTRIBUTES vObjectAttributes;
		InitializeObjectAttributes(&vObjectAttributes, &ustrName, 0, nullptr, nullptr);

		HANDLE hDirectory;
		const auto lStatus = ::NtOpenDirectoryObject(&hDirectory, 0x0F, &vObjectAttributes);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"NtOpenDirectoryObject() 失败。"));
		}
		return Impl_UniqueNtHandle::UniqueNtHandle(hDirectory);
	}
}

}
