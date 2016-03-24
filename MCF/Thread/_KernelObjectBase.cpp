// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_KernelObjectBase.hpp"
#include "../Core/Exception.hpp"
#include "../Core/StringView.hpp"
#include "../Utilities/CopyMoveFill.hpp"
#include <winternl.h>
#include <ntdef.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtOpenDirectoryObject(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) noexcept;

namespace MCF {

namespace {
	inline wchar_t *PrintNumberAsDec(wchar_t *pwcBuffer, unsigned long ulValue) noexcept {
		int nOffset = 0;
		do {
			pwcBuffer[nOffset++] = L"0123456789"[ulValue % 10];
			ulValue /= 10;
		} while(ulValue != 0);

		for(int i = 0, j = nOffset - 1; i < j; ++i, --j){
			std::swap(pwcBuffer[i], pwcBuffer[j]);
		}
		return pwcBuffer + nOffset;
	}
}

namespace Impl_KernelObjectBase {
	Impl_UniqueNtHandle::UniqueNtHandle KernelObjectBase::Y_OpenBaseNamedObjectDirectory(std::uint32_t u32Flags){
		static const auto kSessionPathPrefix = L"\\Sessions\\"_wsv;
		static const auto kBaseNameObjects   = L"\\BaseNamedObjects"_wsv;

		wchar_t awcNameBuffer[128];
		wchar_t *pwcBegin, *pwcEnd;
		if(u32Flags & kGlobal){
			pwcBegin = (wchar_t *)kBaseNameObjects.GetBegin();
			pwcEnd   = (wchar_t *)kBaseNameObjects.GetEnd();
		} else {
			pwcBegin = awcNameBuffer;
			pwcEnd   = awcNameBuffer;

			pwcEnd = Copy(pwcEnd, kSessionPathPrefix.GetBegin(), kSessionPathPrefix.GetEnd());
			pwcEnd = PrintNumberAsDec(pwcEnd, ::WTSGetActiveConsoleSessionId());
			pwcEnd = Copy(pwcEnd, kBaseNameObjects.GetBegin(), kBaseNameObjects.GetEnd());
		}
		const auto ushLengthInBytes = (unsigned short)((char *)pwcEnd - (char *)pwcBegin);

		::UNICODE_STRING ustrName;
		ustrName.Length        = ushLengthInBytes;
		ustrName.MaximumLength = ushLengthInBytes;
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
