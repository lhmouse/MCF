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
		constexpr HANDLE operator()() const {
			return INVALID_HANDLE_VALUE;
		}
		void operator()(HANDLE hFile) const {
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
	FileDelegate(const WideStringObserver &wsoPath, bool bToRead, bool bToWrite, bool bAutoCreate){
		xm_hFile.Reset(::CreateFileW(
			wsoPath.GetNullTerminated<MAX_PATH>().GetData(),
			(bToRead ? GENERIC_READ : 0) | (bToWrite ? GENERIC_WRITE : 0),
			bToWrite ? 0 : FILE_SHARE_READ,
			nullptr,
			(bToWrite && bAutoCreate) ? OPEN_ALWAYS : OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL
		));
		if(!xm_hFile){
			MCF_THROW(::GetLastError(), L"::CreateFileW() 失败。");
		}
	}

public:
	std::uint64_t GetSize() const {
		LARGE_INTEGER liFileSize;
		if(!::GetFileSizeEx(xm_hFile.Get(), &liFileSize)){
			MCF_THROW(::GetLastError(), L"::GetFileSizeEx() 失败。");
		}
		return (std::uint64_t)liFileSize.QuadPart;
	}
	void Resize(std::uint64_t u64NewSize){
		if(u64NewSize > (std::uint64_t)LLONG_MAX){
			MCF_THROW(ERROR_INVALID_PARAMETER, L"调整文件大小时指定的大小无效。");
		}
		LARGE_INTEGER liNewSize;
		liNewSize.QuadPart = (long long)u64NewSize;
		if(!::SetFilePointerEx(xm_hFile.Get(), liNewSize, nullptr, FILE_BEGIN)){
			MCF_THROW(::GetLastError(), L"::SetFilePointerEx() 失败。");
		}
		if(!::SetEndOfFile(xm_hFile.Get())){
			MCF_THROW(::GetLastError(), L"::SetEndOfFile() 失败。");
		}
	}

	DWORD Read(void *pBuffer, DWORD dwBytesToRead, std::uint64_t u64Offset, AsyncProc *pfnAsyncProc) const {
		xApcResult ApcResult;
		ApcResult.dwBytesTransferred = 0;
		ApcResult.dwErrorCode = ERROR_SUCCESS;

		OVERLAPPED Overlapped;
		BZero(Overlapped);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);
		Overlapped.hEvent = (HANDLE)&ApcResult;
		const bool bSucceeds = ::ReadFileEx(xm_hFile.Get(), pBuffer, dwBytesToRead, &Overlapped, &xAioCallback);
		if(!bSucceeds){
			ApcResult.dwErrorCode = ::GetLastError();
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
			MCF_THROW(ApcResult.dwErrorCode, L"::ReadFileEx() 失败。");
		}
		return ApcResult.dwBytesTransferred;
	}
	void Write(std::uint64_t u64Offset, const void *pBuffer, DWORD dwBytesToWrite, AsyncProc *pfnAsyncProc){
		xApcResult ApcResult;
		ApcResult.dwBytesTransferred = 0;
		ApcResult.dwErrorCode = ERROR_SUCCESS;

		OVERLAPPED Overlapped;
		BZero(Overlapped);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);
		Overlapped.hEvent = (HANDLE)&ApcResult;
		const bool bSucceeds = ::WriteFileEx(xm_hFile.Get(), pBuffer, dwBytesToWrite, &Overlapped, &xAioCallback);
		if(!bSucceeds){
			ApcResult.dwErrorCode = ::GetLastError();
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
			MCF_THROW(ApcResult.dwErrorCode, L"::WriteFileEx() 失败。");
		}
	}
};

}

// 静态成员函数。
std::unique_ptr<File> File::Open(const WideStringObserver &wsoPath, bool bToRead, bool bToWrite, bool bAutoCreate){
	return std::unique_ptr<File>(new FileDelegate(wsoPath, bToRead, bToWrite, bAutoCreate));
}
std::unique_ptr<File> File::OpenNoThrow(const WideStringObserver &wsoPath, bool bToRead, bool bToWrite, bool bAutoCreate){
	try {
		return Open(wsoPath, bToRead, bToWrite, bAutoCreate);
	} catch(Exception &e){
		SetWin32LastError(e.ulErrorCode);
		return std::unique_ptr<File>();
	}
}

// 其他非静态成员函数。
std::uint64_t File::GetSize() const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	return ((const FileDelegate *)this)->GetSize();
}
void File::Resize(std::uint64_t u64NewSize){
	ASSERT(dynamic_cast<FileDelegate *>(this));

	((FileDelegate *)this)->Resize(u64NewSize);
}
void File::Clear(){
	Resize(0);
}

std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	std::size_t uBytesRead = 0;
	for(;;){
		const DWORD dwBytesToReadThisTime = Min(0xFFFFF000u, uBytesToRead - uBytesRead);
		const DWORD dwBytesReadThisTime = ((const FileDelegate *)this)->Read(
			(unsigned char *)pBuffer + uBytesRead,
			dwBytesToReadThisTime,
			u64Offset + uBytesRead,
			nullptr
		);
		if(dwBytesReadThisTime == 0){
			break;
		}
		uBytesRead += dwBytesReadThisTime;
	}
	return uBytesRead;
}
std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset, File::AsyncProc &&fnAsyncProc) const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	DWORD dwBytesToReadThisTime = Min(0xFFFFF000u, uBytesToRead);
	std::size_t uBytesRead = ((const FileDelegate *)this)->Read(
		pBuffer,
		dwBytesToReadThisTime,
		u64Offset,
		&fnAsyncProc
	);
	if(uBytesRead == 0){
		return 0;
	}
	for(;;){
		dwBytesToReadThisTime = Min(0xFFFFF000u, uBytesToRead - uBytesRead);
		const DWORD dwBytesReadThisTime = ((const FileDelegate *)this)->Read(
			(unsigned char *)pBuffer + uBytesRead,
			dwBytesToReadThisTime,
			u64Offset + uBytesRead,
			nullptr
		);
		if(dwBytesReadThisTime == 0){
			break;
		}
		uBytesRead += dwBytesReadThisTime;
	}
	return uBytesRead;
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite){
	ASSERT(dynamic_cast<FileDelegate *>(this));

	std::size_t uBytesWritten = 0;
	for(;;){
		const DWORD dwBytesToWriteThisTime = Min(0xFFFFF000u, uBytesToWrite - uBytesWritten);
		if(dwBytesToWriteThisTime == 0){
			break;
		}
		((FileDelegate *)this)->Write(
			u64Offset + uBytesWritten,
			(const unsigned char *)pBuffer + uBytesWritten,
			dwBytesToWriteThisTime,
			nullptr
		);
		uBytesWritten += dwBytesToWriteThisTime;
	}
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite, File::AsyncProc &&fnAsyncProc){
	ASSERT(dynamic_cast<FileDelegate *>(this));

	DWORD dwBytesToWriteThisTime = Min(0xFFFFF000u, uBytesToWrite);
	if(dwBytesToWriteThisTime == 0){
		return;
	}
	((FileDelegate *)this)->Write(
		u64Offset,
		pBuffer,
		dwBytesToWriteThisTime,
		&fnAsyncProc
	);
	std::size_t uBytesWritten = dwBytesToWriteThisTime;
	for(;;){
		dwBytesToWriteThisTime = Min(0xFFFFF000u, uBytesToWrite - uBytesWritten);
		if(dwBytesToWriteThisTime == 0){
			break;
		}
		((FileDelegate *)this)->Write(
			u64Offset + uBytesWritten,
			(const unsigned char *)pBuffer + uBytesWritten,
			dwBytesToWriteThisTime,
			nullptr
		);
		uBytesWritten += dwBytesToWriteThisTime;
	}
}
