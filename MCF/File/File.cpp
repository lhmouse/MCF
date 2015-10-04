// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "File.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Utilities/BinaryOperations.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Utilities/Defer.hpp"
#include "../Thread/Thread.hpp"
#include <winternl.h>
#include <ntdef.h>
#include <ntstatus.h>

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

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS RtlGetFullPathName_UstrEx(const UNICODE_STRING *pFileName, UNICODE_STRING *pStaticBuffer, UNICODE_STRING *pDynamicBuffer,
	UNICODE_STRING **ppWhichBufferIsUsed, SIZE_T *puPrefixChars, BOOLEAN *pbValid, RTL_PATH_TYPE *pePathType, SIZE_T *puBytesRequired);

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReadFile(HANDLE hFile, HANDLE hEvent, PIO_APC_ROUTINE pfnApcRoutine, void *pApcContext, IO_STATUS_BLOCK *pIoStatus,
	void *pBuffer, ULONG ulLength, LARGE_INTEGER *pliOffset, ULONG *pulKey) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWriteFile(HANDLE hFile, HANDLE hEvent, PIO_APC_ROUTINE pfnApcRoutine, void *pApcContext, IO_STATUS_BLOCK *pIoStatus,
	const void *pBuffer, ULONG ulLength, LARGE_INTEGER *pliOffset, ULONG *pulKey) noexcept;

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtFlushBuffersFile(HANDLE hFile, IO_STATUS_BLOCK *pIoStatus) noexcept;

namespace MCF {

void Impl_File::NtHandleCloser::operator()(void *hFile) const noexcept {
	const auto lStatus = ::NtClose(hFile);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"::NtClose() 失败。");
	}
}

namespace {
	__attribute__((__stdcall__, __force_align_arg_pointer__, __aligned__(16)))
	void IoApcCallback(void *pContext, ::IO_STATUS_BLOCK * /* pIoStatus */, ULONG /* ulReserved */) noexcept {
		const auto pbIoPending = static_cast<bool *>(pContext);
		*pbIoPending = false;
	}
}

// 构造函数和析构函数。
File::File(const WideStringObserver &wsoPath, std::uint32_t u32Flags){
	const auto uSize = wsoPath.GetSize() * sizeof(wchar_t);
	if(uSize > UINT16_MAX){
		DEBUG_THROW(SystemError, ERROR_INVALID_PARAMETER, "The path for a file is too long"_rcs);
	}
	::UNICODE_STRING ustrRawPath;
	ustrRawPath.Length              = uSize;
	ustrRawPath.MaximumLength       = uSize;
	ustrRawPath.Buffer              = (PWSTR)wsoPath.GetBegin();

	static constexpr wchar_t kDosPathPrefix[] = LR"(\??\)";
	static constexpr auto kDosPathPrefixSize = sizeof(kDosPathPrefix) - sizeof(wchar_t);

	wchar_t awcStaticStr[MAX_PATH + kDosPathPrefixSize];
	::UNICODE_STRING ustrStaticBuffer;
	ustrStaticBuffer.Length         = 0;
	ustrStaticBuffer.MaximumLength  = sizeof(awcStaticStr);
	ustrStaticBuffer.Buffer         = awcStaticStr;

	::UNICODE_STRING ustrDynamicBuffer;
	ustrDynamicBuffer.Length        = 0;
	ustrDynamicBuffer.MaximumLength = 0;
	ustrDynamicBuffer.Buffer        = nullptr;
	DEFER([&]{ ::RtlFreeUnicodeString(&ustrDynamicBuffer); });

	::UNICODE_STRING *pustrUnprefixedFullPath;
	::RTL_PATH_TYPE ePathType;
	const auto lPathStatus = ::RtlGetFullPathName_UstrEx(&ustrRawPath, &ustrStaticBuffer, &ustrDynamicBuffer, &pustrUnprefixedFullPath, nullptr, nullptr, &ePathType, nullptr);
	if(!NT_SUCCESS(lPathStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lPathStatus), "RtlGetFullPathName_UstrEx"_rcs);
	}

	auto uPrefixedPathSize = pustrUnprefixedFullPath->Length;
	auto pbyPrefixedPathBuffer = reinterpret_cast<unsigned char *>(pustrUnprefixedFullPath->Buffer);
	DEFER([&]{ if(pbyPrefixedPathBuffer != reinterpret_cast<unsigned char *>(pustrUnprefixedFullPath->Buffer)){ ::operator delete[](pbyPrefixedPathBuffer); } });

	if((RtlPathTypeDriveAbsolute <= ePathType) && (ePathType <= RtlPathTypeRelative)){
		uPrefixedPathSize += kDosPathPrefixSize;
		if(static_cast<std::size_t>(pustrUnprefixedFullPath->MaximumLength) - pustrUnprefixedFullPath->Length <= uPrefixedPathSize){
			pbyPrefixedPathBuffer = reinterpret_cast<unsigned char *>(pustrUnprefixedFullPath->Buffer);
		} else {
			pbyPrefixedPathBuffer = static_cast<unsigned char *>(::operator new[](uPrefixedPathSize));
		}
		std::memmove(pbyPrefixedPathBuffer + kDosPathPrefixSize, pustrUnprefixedFullPath->Buffer, pustrUnprefixedFullPath->Length);
		std::memcpy(pbyPrefixedPathBuffer, kDosPathPrefix, kDosPathPrefixSize);
	}

	::ACCESS_MASK dwDesiredAccess = 0;
	if(u32Flags & kToRead){
		dwDesiredAccess |= FILE_GENERIC_READ;
	}
	if(u32Flags & kToWrite){
		dwDesiredAccess |= FILE_GENERIC_WRITE;
	}

	::UNICODE_STRING ustrPrefixedFullPath;
	ustrPrefixedFullPath.Length        = uPrefixedPathSize;
	ustrPrefixedFullPath.MaximumLength = uPrefixedPathSize;
	ustrPrefixedFullPath.Buffer        = (PWSTR)pbyPrefixedPathBuffer;
	::OBJECT_ATTRIBUTES vObjectAttributes;
	InitializeObjectAttributes(&vObjectAttributes, &ustrPrefixedFullPath, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

	::IO_STATUS_BLOCK vIoStatus;

	DWORD dwSharedAccess;
	if(u32Flags & kToWrite){
		dwSharedAccess = 0;
	} else {
		dwSharedAccess = FILE_SHARE_READ;
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

	DWORD dwCreateOptions = FILE_NON_DIRECTORY_FILE | FILE_RANDOM_ACCESS;
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

	HANDLE hFile;
	const auto lStatus = ::NtCreateFile(&hFile, dwDesiredAccess, &vObjectAttributes, &vIoStatus, nullptr, FILE_ATTRIBUTE_NORMAL, dwSharedAccess, dwCreateDisposition, dwCreateOptions, nullptr, 0);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtCreateFile"_rcs);
	}
	x_hFile.Reset(hFile);
}

// 其他非静态成员函数。
bool File::IsOpen() const noexcept {
	return !!x_hFile;
}
void File::Open(const WideStringObserver &wsoPath, std::uint32_t u32Flags){
	File(wsoPath, u32Flags).Swap(*this);
}
bool File::OpenNoThrow(const WideStringObserver &wsoPath, std::uint32_t u32Flags){
	try {
		Open(wsoPath, u32Flags);
		return true;
	} catch(SystemError &e){
		::SetLastError(e.GetCode());
		return false;
	}
}
void File::Close(){
	if(!x_hFile){
		return;
	}

	Flush();
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
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtQueryInformationFile"_rcs);
	}
	return static_cast<std::uint64_t>(vStandardInfo.EndOfFile.QuadPart);
}
void File::Resize(std::uint64_t u64NewSize){
	if(!x_hFile){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No file opened"_rcs);
	}

	if(u64NewSize >= static_cast<std::uint64_t>(INT64_MAX)){
		DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, "File size is too large"_rcs);
	}

	::IO_STATUS_BLOCK vIoStatus;
	::FILE_END_OF_FILE_INFORMATION vEofInfo;
	vEofInfo.EndOfFile.QuadPart = static_cast<std::int64_t>(u64NewSize);
	const auto lStatus = ::NtSetInformationFile(x_hFile.Get(), &vIoStatus, &vEofInfo, sizeof(vEofInfo), FileEndOfFileInformation);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtSetInformationFile"_rcs);
	}
}
void File::Clear(){
	Resize(0);
}

std::size_t File::Read(void *pBuffer, std::uint32_t u32BytesToRead, std::uint64_t u64Offset,
	FunctionObserver<void ()> fnAsyncProc, FunctionObserver<void ()> fnCompleteCallback) const
{
	if(!x_hFile){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No file opened"_rcs);
	}

	if(u64Offset >= static_cast<std::uint64_t>(INT64_MAX)){
		DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, "File offset is too large"_rcs);
	}

	bool bIoPending = true;
	::IO_STATUS_BLOCK vIoStatus;
	::LARGE_INTEGER liOffset;
	liOffset.QuadPart = static_cast<std::int64_t>(u64Offset);
	const auto lStatus = ::NtReadFile(x_hFile.Get(), nullptr, &IoApcCallback, &bIoPending, &vIoStatus, pBuffer, u32BytesToRead, &liOffset, nullptr);
	if(fnAsyncProc){
		fnAsyncProc();
	}
	if(lStatus == STATUS_END_OF_FILE){
		vIoStatus.Information = 0;
		goto jEof;
	}
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtReadFile"_rcs);
	}
	do {
		Thread::AlertableSleep();
	} while(bIoPending);

jEof:
	if(fnCompleteCallback){
		fnCompleteCallback();
	}
	return vIoStatus.Information;
}
std::size_t File::Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite,
	FunctionObserver<void ()> fnAsyncProc, FunctionObserver<void ()> fnCompleteCallback)
{
	if(!x_hFile){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No file opened"_rcs);
	}

	if(u64Offset >= static_cast<std::uint64_t>(INT64_MAX)){
		DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, "File offset is too large"_rcs);
	}

	bool bIoPending = true;
	::IO_STATUS_BLOCK vIoStatus;
	::LARGE_INTEGER liOffset;
	liOffset.QuadPart = static_cast<std::int64_t>(u64Offset);
	const auto lStatus = ::NtWriteFile(x_hFile.Get(), nullptr, &IoApcCallback, &bIoPending, &vIoStatus, pBuffer, u32BytesToWrite, &liOffset, nullptr);
	if(fnAsyncProc){
		fnAsyncProc();
	}
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtWriteFile"_rcs);
	}
	do {
		Thread::AlertableSleep();
	} while(bIoPending);

	if(fnCompleteCallback){
		fnCompleteCallback();
	}
	return vIoStatus.Information;
}
void File::Flush() const {
	if(!x_hFile){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No file opened"_rcs);
	}

	::IO_STATUS_BLOCK vIoStatus;
	const auto lStatus = ::NtFlushBuffersFile(x_hFile.Get(), &vIoStatus);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtFlushBuffersFile"_rcs);
	}
}

}
