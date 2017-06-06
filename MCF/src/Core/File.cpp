// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "File.hpp"
#include "Exception.hpp"
#include "String.hpp"
#include "BinaryOperations.hpp"
#include "Defer.hpp"
#include "MinMax.hpp"
#include <MCFCRT/env/mcfwin.h>
#include <ntdef.h>
#include <ntstatus.h>

extern "C" {

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtOpenDirectoryObject(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) noexcept;

typedef enum tagRTL_PATH_TYPE {
	RtlPathTypeUnknown,
	RtlPathTypeUncAbsolute,
	RtlPathTypeDriveAbsolute,
	RtlPathTypeDriveRelative,
	RtlPathTypeRooted,
	RtlPathTypeRelative,
	RtlPathTypeLocalDevice,
	RtlPathTypeRootLocalDevice,
} RTL_PATH_TYPE;

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS RtlGetFullPathName_UstrEx(const UNICODE_STRING *pFileName, UNICODE_STRING *pStaticBuffer, UNICODE_STRING *pDynamicBuffer,
	UNICODE_STRING **ppWhichBufferIsUsed, SIZE_T *puPrefixChars, BOOLEAN *pbValid, RTL_PATH_TYPE *pePathType, SIZE_T *puBytesRequired);

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtReadFile(HANDLE hFile, HANDLE hEvent, PIO_APC_ROUTINE pfnApcRoutine, void *pApcContext, IO_STATUS_BLOCK *pIoStatus,
	void *pBuffer, ULONG ulLength, const LARGE_INTEGER *pliOffset, const ULONG *pulKey) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtWriteFile(HANDLE hFile, HANDLE hEvent, PIO_APC_ROUTINE pfnApcRoutine, void *pApcContext, IO_STATUS_BLOCK *pIoStatus,
	const void *pBuffer, ULONG ulLength, const LARGE_INTEGER *pliOffset, const ULONG *pulKey) noexcept;

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtFlushBuffersFile(HANDLE hFile, IO_STATUS_BLOCK *pIoStatus) noexcept;

}

namespace MCF {

File::File(const WideStringView &wsvPath, std::uint32_t u32Flags){
	constexpr wchar_t kDosDevicePath[] = { L'\\', L'?', L'?', L'\\' };

	const auto uSize = wsvPath.GetSize() * sizeof(wchar_t);
	if(uSize > USHRT_MAX){
		MCF_THROW(Exception, ERROR_BUFFER_OVERFLOW, Rcntws::View(L"File: 文件名太长。"));
	}
	::UNICODE_STRING ustrRawPath;
	ustrRawPath.Length              = (USHORT)uSize;
	ustrRawPath.MaximumLength       = (USHORT)uSize;
	ustrRawPath.Buffer              = (PWSTR)wsvPath.GetBegin();

	wchar_t awcStaticStr[MAX_PATH];
	::UNICODE_STRING ustrStaticBuffer;
	ustrStaticBuffer.Length         = 0;
	ustrStaticBuffer.MaximumLength  = sizeof(awcStaticStr);
	ustrStaticBuffer.Buffer         = awcStaticStr;

	::UNICODE_STRING ustrDynamicBuffer;
	ustrDynamicBuffer.Length        = 0;
	ustrDynamicBuffer.MaximumLength = 0;
	ustrDynamicBuffer.Buffer        = nullptr;
	const auto vFreeDynamicBuffer = Defer([&]{ ::RtlFreeUnicodeString(&ustrDynamicBuffer); });

	::UNICODE_STRING *pustrFullPath;
	Impl_UniqueNtHandle::UniqueNtHandle hRootDirectory;
	if((ustrRawPath.Length >= sizeof(kDosDevicePath)) && (std::memcmp(ustrRawPath.Buffer, kDosDevicePath, sizeof(kDosDevicePath)) == 0)){
		pustrFullPath = &ustrRawPath;
	} else {
		::RTL_PATH_TYPE ePathType;
		const auto lPathStatus = ::RtlGetFullPathName_UstrEx(&ustrRawPath, &ustrStaticBuffer, &ustrDynamicBuffer, &pustrFullPath, nullptr, nullptr, &ePathType, nullptr);
		if(!NT_SUCCESS(lPathStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lPathStatus), Rcntws::View(L"File: RtlGetFullPathName_UstrEx() 失败。"));
		}
		if((RtlPathTypeDriveAbsolute <= ePathType) && (ePathType <= RtlPathTypeRelative)){
			::UNICODE_STRING ustrName;
			ustrName.Length        = sizeof(kDosDevicePath);
			ustrName.MaximumLength = sizeof(kDosDevicePath);
			ustrName.Buffer        = (PWSTR)kDosDevicePath;

			::OBJECT_ATTRIBUTES vObjectAttributes;
			InitializeObjectAttributes(&vObjectAttributes, &ustrName, 0, nullptr, nullptr);

			HANDLE hTemp;
			const auto lStatus = ::NtOpenDirectoryObject(&hTemp, 0x0F, &vObjectAttributes);
			if(!NT_SUCCESS(lStatus)){
				MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"File: NtOpenDirectoryObject() 失败。"));
			}
			hRootDirectory.Reset(hTemp);
		}
	}

	::ACCESS_MASK dwDesiredAccess = SYNCHRONIZE;
	if(u32Flags & kToRead){
		dwDesiredAccess |= FILE_GENERIC_READ;
	}
	if(u32Flags & kToWrite){
		dwDesiredAccess |= FILE_GENERIC_WRITE;
	}

	::OBJECT_ATTRIBUTES vObjectAttributes;
	InitializeObjectAttributes(&vObjectAttributes, pustrFullPath, 0 /* OBJ_CASE_INSENSITIVE */, hRootDirectory.Get(), nullptr);

	::IO_STATUS_BLOCK vIoStatus;

	DWORD dwSharedAccess;
	if(u32Flags & kToWrite){
		dwSharedAccess = 0;
	} else {
		dwSharedAccess = FILE_SHARE_READ;
	}
	if(u32Flags & kSharedRead){
		dwSharedAccess |= FILE_SHARE_READ;
	}
	if(u32Flags & kSharedWrite){
		dwSharedAccess |= FILE_SHARE_WRITE;
	}
	if(u32Flags & kSharedDelete){
		dwSharedAccess |= FILE_SHARE_DELETE;
	}

	DWORD dwCreateDisposition;
	if(u32Flags & kToWrite){
		if(u32Flags & kDontCreate){
			dwCreateDisposition = FILE_OPEN;
		} else if(u32Flags & kFailIfExists){
			dwCreateDisposition = FILE_CREATE;
		} else {
			dwCreateDisposition = FILE_OPEN_IF;
		}
	} else {
		dwCreateDisposition = FILE_OPEN;
	}

	DWORD dwCreateOptions = FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_RANDOM_ACCESS;
	if(u32Flags & kNoBuffering){
		dwCreateOptions |= FILE_NO_INTERMEDIATE_BUFFERING;
	}
	if(u32Flags & kWriteThrough){
		dwCreateOptions |= FILE_WRITE_THROUGH;
	}
	if(u32Flags & kDeleteOnClose){
		dwDesiredAccess |= DELETE;
		dwCreateOptions |= FILE_DELETE_ON_CLOSE;
	}

	HANDLE hTemp;
	const auto lStatus = ::NtCreateFile(&hTemp, dwDesiredAccess, &vObjectAttributes, &vIoStatus, nullptr, FILE_ATTRIBUTE_NORMAL, dwSharedAccess, dwCreateDisposition, dwCreateOptions, nullptr, 0);
	if(!NT_SUCCESS(lStatus)){
		MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"File: NtCreateFile() 失败。"));
	}
	x_hFile.Reset(hTemp);
}

void File::Open(const WideStringView &wsvPath, std::uint32_t u32Flags){
	File(wsvPath, u32Flags).Swap(*this);
}
bool File::OpenNothrow(const WideStringView &wsvPath, std::uint32_t u32Flags){
	try {
		Open(wsvPath, u32Flags);
		return true;
	} catch(Exception &e){
		::SetLastError(e.GetErrorCode());
		return false;
	}
}
void File::Close() noexcept {
	File().Swap(*this);
}

std::uint64_t File::GetSize() const {
	if(!x_hFile){
		return 0;
	}

	::IO_STATUS_BLOCK vIoStatus;
	::FILE_STANDARD_INFORMATION vStandardInfo;
	const auto lStatus = ::NtQueryInformationFile(x_hFile.Get(), &vIoStatus, &vStandardInfo, sizeof(vStandardInfo), FileStandardInformation);
	if(!NT_SUCCESS(lStatus)){
		MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"File: NtQueryInformationFile() 失败。"));
	}
	return static_cast<std::uint64_t>(vStandardInfo.EndOfFile.QuadPart);
}
void File::Resize(std::uint64_t u64NewSize){
	if(!x_hFile){
		MCF_THROW(Exception, ERROR_INVALID_HANDLE, Rcntws::View(L"File: 尚未打开任何文件。"));
	}

	if(u64NewSize >= static_cast<std::uint64_t>(INT64_MAX)){
		MCF_THROW(Exception, ERROR_INVALID_PARAMETER, Rcntws::View(L"File: 文件大小过大。"));
	}

	::IO_STATUS_BLOCK vIoStatus;
	::FILE_END_OF_FILE_INFORMATION vEofInfo;
	vEofInfo.EndOfFile.QuadPart = static_cast<std::int64_t>(u64NewSize);
	const auto lStatus = ::NtSetInformationFile(x_hFile.Get(), &vIoStatus, &vEofInfo, sizeof(vEofInfo), FileEndOfFileInformation);
	if(!NT_SUCCESS(lStatus)){
		MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"File: NtSetInformationFile() 失败。"));
	}
}
void File::Clear(){
	Resize(0);
}

std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const {
	if(!x_hFile){
		MCF_THROW(Exception, ERROR_INVALID_HANDLE, Rcntws::View(L"File: 尚未打开任何文件。"));
	}

	if(u64Offset >= static_cast<std::uint64_t>(INT64_MAX)){
		MCF_THROW(Exception, ERROR_SEEK, Rcntws::View(L"File: 文件偏移量太大。"));
	}

	::IO_STATUS_BLOCK vIoStatus;
	vIoStatus.Information = 0;

	::LARGE_INTEGER liOffset;
	liOffset.QuadPart = static_cast<std::int64_t>(u64Offset);
	const auto lStatus = ::NtReadFile(x_hFile.Get(), nullptr, nullptr, nullptr, &vIoStatus, pBuffer, static_cast<ULONG>(Min(uBytesToRead, static_cast<ULONG>(-1))), &liOffset, nullptr);
	if((lStatus != STATUS_END_OF_FILE) && !NT_SUCCESS(lStatus)){
		MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"File: NtReadFile() 失败。"));
	}
	return vIoStatus.Information;
}
std::size_t File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite){
	if(!x_hFile){
		MCF_THROW(Exception, ERROR_INVALID_HANDLE, Rcntws::View(L"File: 尚未打开任何文件。"));
	}

	if(u64Offset >= static_cast<std::uint64_t>(INT64_MAX)){
		MCF_THROW(Exception, ERROR_SEEK, Rcntws::View(L"File: 文件偏移量太大。"));
	}

	::IO_STATUS_BLOCK vIoStatus;
	vIoStatus.Information = 0;

	::LARGE_INTEGER liOffset;
	liOffset.QuadPart = static_cast<std::int64_t>(u64Offset);
	const auto lStatus = ::NtWriteFile(x_hFile.Get(), nullptr, nullptr, nullptr, &vIoStatus, pBuffer, static_cast<ULONG>(Min(uBytesToWrite, static_cast<ULONG>(-1))), &liOffset, nullptr);
	if(!NT_SUCCESS(lStatus)){
		MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"File: NtWriteFile() 失败。"));
	}
	return vIoStatus.Information;
}
void File::Flush(){
	if(!x_hFile){
		MCF_THROW(Exception, ERROR_INVALID_HANDLE, Rcntws::View(L"File: 尚未打开任何文件。"));
	}

	::IO_STATUS_BLOCK vIoStatus;
	const auto lStatus = ::NtFlushBuffersFile(x_hFile.Get(), &vIoStatus);
	if(!NT_SUCCESS(lStatus)){
		MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"File: NtFlushBuffersFile() 失败。"));
	}
}

}
