// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "_crt_process.h"
#include "mutex.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include <winternl.h>
#include <ntdef.h>

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtCreateSection(HANDLE *phSection, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes,
	const LARGE_INTEGER *pliMaximumSize, ULONG ulSectionPageProtection, ULONG ulAllocationAttributes, HANDLE hFile);

typedef enum tagSECTION_INHERIT {
	ViewShare = 1,
	ViewUnmap = 2,
} SECTION_INHERIT;

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtMapViewOfSection(HANDLE hSection, HANDLE hProcess, void **ppBaseAddress, ULONG_PTR uZeroBits, SIZE_T uCommitSize,
	LARGE_INTEGER *pliSectionOffset, SIZE_T *puViewSize, SECTION_INHERIT eInheritDisposition, ULONG ulAllocationType, ULONG ulWin32Protect);
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtUnmapViewOfSection(HANDLE hProcess, void *pBaseAddress);


bool __MCFCRT_ProcessInit(void){
	return true;
}
void __MCFCRT_ProcessUninit(void){
}

/*
static _MCFCRT_Mutex                      g_vMutex  = { 0 };
static __MCFCRT_ProcessEnvironmentBlock * g_pCrtPeb = nullptr;

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
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelObjectBase:: NtOpenDirectoryObject() 失败。"));
		}
		return Impl_UniqueNtHandle::UniqueNtHandle(hDirectory);
	}

	if(uInitCount >= static_cast<std::size_t>(LONG_MAX)){
		MCF_THROW(Exception, ERROR_INVALID_PARAMETER, Rcntws::View(L"KernelSemaphore: 信号量初始计数太大。"));
	}

	Impl_UniqueNtHandle::UniqueNtHandle hRootDirectory;
	::OBJECT_ATTRIBUTES vObjectAttributes;

	const auto uNameSize = wsvName.GetSize() * sizeof(wchar_t);
	if(uNameSize == 0){
		InitializeObjectAttributes(&vObjectAttributes, nullptr, 0, nullptr, nullptr);
	} else {
		if(uNameSize > USHRT_MAX){
			MCF_THROW(Exception, ERROR_BUFFER_OVERFLOW, Rcntws::View(L"KernelSemaphore: 内核对象的路径太长。"));
		}
		::UNICODE_STRING ustrObjectName;
		ustrObjectName.Length        = (USHORT)uNameSize;
		ustrObjectName.MaximumLength = (USHORT)uNameSize;
		ustrObjectName.Buffer        = (PWSTR)wsvName.GetBegin();

		ULONG ulAttributes;
		if(u32Flags & kFailIfExists){
			ulAttributes = 0;
		} else {
			ulAttributes = OBJ_OPENIF;
		}

		hRootDirectory = Y_OpenBaseNamedObjectDirectory(u32Flags);

		InitializeObjectAttributes(&vObjectAttributes, &ustrObjectName, ulAttributes, hRootDirectory.Get(), nullptr);
	}

	HANDLE hTemp;
	if(u32Flags & kDontCreate){
		const auto lStatus = ::NtOpenSemaphore(&hTemp, SEMAPHORE_ALL_ACCESS, &vObjectAttributes);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelSemaphore: NtOpenSemaphore() 失败。"));
		}
	} else {
		const auto lStatus = ::NtCreateSemaphore(&hTemp, SEMAPHORE_ALL_ACCESS, &vObjectAttributes, static_cast<LONG>(uInitCount), LONG_MAX);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"KernelSemaphore: NtCreateSemaphore() 失败。"));
		}
	}
	Impl_UniqueNtHandle::UniqueNtHandle hSemaphore(hTemp);

	return hSemaphore;

extern bool __MCFCRT_ProcessInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_ProcessUninit(void) _MCFCRT_NOEXCEPT;

typedef struct __MCFCRT_tagProcessEnvironmentBlock {
	volatile _MCFCRT_STD size_t __uRefCount;
} __MCFCRT_ProcessEnvironmentBlock;

__attribute__((__returns_nonnull__))
extern __MCFCRT_ProcessEnvironmentBlock *__MCFCRT_LockProcessEnvironmentBlock(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_UnlockProcessEnvironmentBlock(__MCFCRT_ProcessEnvironmentBlock *__pCrtPeb) _MCFCRT_NOEXCEPT;

*/
