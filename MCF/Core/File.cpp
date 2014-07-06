// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "File.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
#include "Utilities.hpp"
#include <exception>
#include <list>
using namespace MCF;

namespace {

class FileDelegate : CONCRETE(File) {
private:
	struct xFileCloser {
		constexpr HANDLE operator()() const noexcept {
			return INVALID_HANDLE_VALUE;
		}
		void operator()(HANDLE hFile) const noexcept {
			::CloseHandle(hFile);
		}
	};

	struct xApcResult {
		DWORD dwBytesTransferred;
		DWORD dwErrorCode;
	};

private:
	static void __stdcall xAioCallback(DWORD dwErrorCode, DWORD dwBytesTransferred, LPOVERLAPPED pOverlapped) noexcept {
		const auto pApcResult = (xApcResult *)pOverlapped->hEvent;
		pApcResult->dwErrorCode = dwErrorCode;
		pApcResult->dwBytesTransferred = dwBytesTransferred;
	}

private:
	UniqueHandle<xFileCloser> xm_hFile;

public:
	std::pair<unsigned long, WideStringObserver> Open(const wchar_t *pwszPath, std::uint32_t u32Flags) noexcept {
		DWORD dwCreateDisposition;
		if(u32Flags & TO_WRITE){
			if(u32Flags & NO_CREATE){
				dwCreateDisposition = OPEN_EXISTING;
			} else if(u32Flags & FAIL_IF_EXISTS){
				dwCreateDisposition = CREATE_NEW;
			} else {
				dwCreateDisposition = OPEN_ALWAYS;
			}
		} else {
			dwCreateDisposition = OPEN_EXISTING;
		}

		DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
		if(u32Flags & NO_BUFFERING){
			dwFlagsAndAttributes |= FILE_FLAG_NO_BUFFERING;
		}
		if(u32Flags & WRITE_THROUGH){
			dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
		}
		if(u32Flags & DEL_ON_CLOSE){
			dwFlagsAndAttributes |= FILE_FLAG_DELETE_ON_CLOSE;
		}

		xm_hFile.Reset(::CreateFileW(
			pwszPath,
			((u32Flags & TO_READ) ? GENERIC_READ : 0) | ((u32Flags & TO_WRITE) ? GENERIC_WRITE : 0),
			(u32Flags & TO_WRITE) ? 0 : FILE_SHARE_READ,
			nullptr,
			dwCreateDisposition,
			dwFlagsAndAttributes,
			NULL
		));
		if(!xm_hFile){
			return std::make_pair(::GetLastError(), L"::CreateFileW() 失败。"_wso);
		}
		return std::make_pair((unsigned long)ERROR_SUCCESS, nullptr);
	}

	std::uint64_t GetSize() const {
		ASSERT(xm_hFile);

		LARGE_INTEGER liFileSize;
		if(!::GetFileSizeEx(xm_hFile.Get(), &liFileSize)){
			MCF_THROW(::GetLastError(), L"::GetFileSizeEx() 失败。"_wso);
		}
		return (std::uint64_t)liFileSize.QuadPart;
	}
	void Resize(std::uint64_t u64NewSize){
		ASSERT(xm_hFile);

		if(u64NewSize > (std::uint64_t)LLONG_MAX){
			MCF_THROW(ERROR_INVALID_PARAMETER, L"调整文件大小时指定的大小无效。"_wso);
		}
		LARGE_INTEGER liNewSize;
		liNewSize.QuadPart = (long long)u64NewSize;
		if(!::SetFilePointerEx(xm_hFile.Get(), liNewSize, nullptr, FILE_BEGIN)){
			MCF_THROW(::GetLastError(), L"::SetFilePointerEx() 失败。"_wso);
		}
		if(!::SetEndOfFile(xm_hFile.Get())){
			MCF_THROW(::GetLastError(), L"::SetEndOfFile() 失败。"_wso);
		}
	}

	UniqueId GetUniqueId() const {
		ASSERT(xm_hFile);

		BY_HANDLE_FILE_INFORMATION vFileInfo;
		if(!::GetFileInformationByHandle(xm_hFile.Get(), &vFileInfo)){
			MCF_THROW(::GetLastError(), L"::GetFileInformationByHandle() 失败。"_wso);
		}
		UniqueId vIdRet;
		vIdRet.u32VolumeSN	= vFileInfo.dwVolumeSerialNumber;
		vIdRet.u32IndexLow	= vFileInfo.nFileIndexLow;
		vIdRet.u32IndexHigh	= vFileInfo.nFileIndexHigh;
		vIdRet.u32Reserved	= 0;
		return std::move(vIdRet);
	}

	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset, const std::function<void ()> *pfnAsyncProc) const {
		ASSERT(xm_hFile);

		auto dwBytesToReadThisTime = (DWORD)Min(0xFFFFF000u, uBytesToRead);

		xApcResult vApcResult;
		vApcResult.dwBytesTransferred = 0;
		vApcResult.dwErrorCode = ERROR_SUCCESS;

		OVERLAPPED vOverlapped;
		BZero(vOverlapped);
		vOverlapped.Offset = u64Offset;
		vOverlapped.OffsetHigh = (u64Offset >> 32);
		vOverlapped.hEvent = (HANDLE)&vApcResult;
		const bool bSucceeds = ::ReadFileEx(xm_hFile.Get(), pBuffer, dwBytesToReadThisTime, &vOverlapped, &xAioCallback);
		if(!bSucceeds){
			vApcResult.dwErrorCode = ::GetLastError();
		}
		std::exception_ptr ep;
		if(pfnAsyncProc){
			try {
				(*pfnAsyncProc)();
			} catch(...){
				ep = std::current_exception();
			}
		}
		if(bSucceeds){
			::SleepEx(INFINITE, TRUE);
		}
		if(ep){
			std::rethrow_exception(ep);
		}
		if(!bSucceeds){
			MCF_THROW(vApcResult.dwErrorCode, L"::ReadFileEx() 失败。"_wso);
		}

		std::size_t uBytesRead = vApcResult.dwBytesTransferred;
		while((uBytesRead < uBytesToRead) && (vApcResult.dwBytesTransferred == dwBytesToReadThisTime)){
			dwBytesToReadThisTime = Min(0xFFFFF000u, uBytesToRead - uBytesRead);

			BZero(vOverlapped);
			const auto u64NewOffset = u64Offset + uBytesRead;
			vOverlapped.Offset = u64NewOffset;
			vOverlapped.OffsetHigh = (u64NewOffset >> 32);
			vOverlapped.hEvent = (HANDLE)&vApcResult;
			if(!::ReadFileEx(
				xm_hFile.Get(), (unsigned char *)pBuffer + uBytesRead, dwBytesToReadThisTime,
				&vOverlapped, &xAioCallback
			)){
				MCF_THROW(vApcResult.dwErrorCode, L"::ReadFileEx() 失败。"_wso);
			}
			::SleepEx(INFINITE, TRUE);

			if(vApcResult.dwErrorCode != ERROR_SUCCESS){
				if(vApcResult.dwErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				MCF_THROW(vApcResult.dwErrorCode, L"::ReadFileEx() 失败。"_wso);
			}
			uBytesRead += vApcResult.dwBytesTransferred;
		}
		return uBytesRead;
	}
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite, const std::function<void ()> *pfnAsyncProc){
		ASSERT(xm_hFile);

		auto dwBytesToWriteThisTime = (DWORD)Min(0xFFFFF000u, uBytesToWrite);

		xApcResult vApcResult;
		vApcResult.dwBytesTransferred = 0;
		vApcResult.dwErrorCode = ERROR_SUCCESS;

		OVERLAPPED vOverlapped;
		BZero(vOverlapped);
		vOverlapped.Offset = (DWORD)u64Offset;
		vOverlapped.OffsetHigh = (DWORD)(u64Offset >> 32);
		vOverlapped.hEvent = (HANDLE)&vApcResult;
		const bool bSucceeds = ::WriteFileEx(xm_hFile.Get(), pBuffer, dwBytesToWriteThisTime, &vOverlapped, &xAioCallback);
		if(!bSucceeds){
			vApcResult.dwErrorCode = ::GetLastError();
		}
		std::exception_ptr ep;
		if(pfnAsyncProc){
			try {
				(*pfnAsyncProc)();
			} catch(...){
				ep = std::current_exception();
			}
		}
		if(bSucceeds){
			::SleepEx(INFINITE, TRUE);
		}
		if(ep){
			std::rethrow_exception(ep);
		}
		if(!bSucceeds){
			MCF_THROW(vApcResult.dwErrorCode, L"::WriteFileEx() 失败。"_wso);
		}

		std::size_t uBytesWritten = vApcResult.dwBytesTransferred;
		while((uBytesWritten < uBytesToWrite) && (vApcResult.dwBytesTransferred == dwBytesToWriteThisTime)){
			dwBytesToWriteThisTime = Min(0xFFFFF000u, uBytesToWrite - uBytesWritten);

			BZero(vOverlapped);
			const auto u64NewOffset = u64Offset + uBytesWritten;
			vOverlapped.Offset = (DWORD)u64NewOffset;
			vOverlapped.OffsetHigh = (DWORD)(u64NewOffset >> 32);
			vOverlapped.hEvent = (HANDLE)&vApcResult;
			if(!::WriteFileEx(
				xm_hFile.Get(), (const unsigned char *)pBuffer + uBytesWritten, dwBytesToWriteThisTime,
				&vOverlapped, &xAioCallback
			)){
				MCF_THROW(vApcResult.dwErrorCode, L"::WriteFileEx() 失败。"_wso);
			}
			::SleepEx(INFINITE, TRUE);

			if(vApcResult.dwErrorCode != ERROR_SUCCESS){
				MCF_THROW(vApcResult.dwErrorCode, L"::WriteFileEx() 失败。"_wso);
			}
			uBytesWritten += vApcResult.dwBytesTransferred;
		}
	}

	void Flush() const {
		ASSERT(xm_hFile);

		if(!::FlushFileBuffers(xm_hFile.Get())){
			MCF_THROW(::GetLastError(), L"::WriteFileEx() 失败。"_wso);
		}
	}
};

}

// 静态成员函数。
std::unique_ptr<File> File::Open(const WideStringObserver &wsoPath, std::uint32_t u32Flags){
	auto pFile = std::make_unique<FileDelegate>();
	const auto vResult = pFile->Open(wsoPath.GetNullTerminated<MAX_PATH>().GetData(), u32Flags);
	if(vResult.first != ERROR_SUCCESS){
		MCF_THROW(vResult.first, vResult.second);
	}
	return std::move(pFile);
}
std::unique_ptr<File> File::Open(const WideString &wcsPath, std::uint32_t u32Flags){
	auto pFile = std::make_unique<FileDelegate>();
	const auto vResult = pFile->Open(wcsPath.GetCStr(), u32Flags);
	if(vResult.first != ERROR_SUCCESS){
		MCF_THROW(vResult.first, vResult.second);
	}
	return std::move(pFile);
}

std::unique_ptr<File> File::OpenNoThrow(const WideStringObserver &wsoPath, std::uint32_t u32Flags){
	auto pFile = std::make_unique<FileDelegate>();
	const auto vResult = pFile->Open(wsoPath.GetNullTerminated<MAX_PATH>().GetData(), u32Flags);
	if(vResult.first != ERROR_SUCCESS){
		::SetLastError(vResult.first);
		return nullptr;
	}
	return std::move(pFile);
}
std::unique_ptr<File> File::OpenNoThrow(const WideString &wcsPath, std::uint32_t u32Flags){
	auto pFile = std::make_unique<FileDelegate>();
	const auto vResult = pFile->Open(wcsPath.GetCStr(), u32Flags);
	if(vResult.first != ERROR_SUCCESS){
		::SetLastError(vResult.first);
		return nullptr;
	}
	return std::move(pFile);
}

// 其他非静态成员函数。
std::uint64_t File::GetSize() const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	return ((const FileDelegate *)this)->GetSize();
}
void File::Resize(std::uint64_t u64NewSize){
	ASSERT(dynamic_cast<FileDelegate *>(this));

	static_cast<FileDelegate *>(this)->Resize(u64NewSize);
}
void File::Clear(){
	Resize(0);
	Flush();
}

File::UniqueId File::GetUniqueId() const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	return ((const FileDelegate *)this)->GetUniqueId();
}

std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	return ((const FileDelegate *)this)->Read(pBuffer, uBytesToRead, u64Offset, nullptr);
}
std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset, const File::std::function<void ()> &fnAsyncProc) const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	return ((const FileDelegate *)this)->Read(pBuffer, uBytesToRead, u64Offset, &fnAsyncProc);
}

void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite){
	ASSERT(dynamic_cast<FileDelegate *>(this));

	static_cast<FileDelegate *>(this)->Write(u64Offset, pBuffer, uBytesToWrite, nullptr);
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite, const File::std::function<void ()> &fnAsyncProc){
	ASSERT(dynamic_cast<FileDelegate *>(this));

	static_cast<FileDelegate *>(this)->Write(u64Offset, pBuffer, uBytesToWrite, &fnAsyncProc);
}

void File::Flush() const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	static_cast<const FileDelegate *>(this)->Flush();
}
