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

	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset, AsyncProc *pfnAsyncProc) const {
		DWORD dwBytesToReadThisTime = std::min<std::size_t>(0xFFFFF000u, uBytesToRead);

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
			MCF_THROW(vApcResult.dwErrorCode, L"::ReadFileEx() 失败。");
		}

		std::size_t uBytesRead = vApcResult.dwBytesTransferred;
		while((uBytesRead < uBytesToRead) && (vApcResult.dwBytesTransferred == dwBytesToReadThisTime)){
			dwBytesToReadThisTime = std::min<std::size_t>(0xFFFFF000u, uBytesToRead - uBytesRead);

			BZero(vOverlapped);
			const auto u64NewOffset = u64Offset + uBytesRead;
			vOverlapped.Offset = u64NewOffset;
			vOverlapped.OffsetHigh = (u64NewOffset >> 32);
			vOverlapped.hEvent = (HANDLE)&vApcResult;
			if(!::ReadFileEx(
				xm_hFile.Get(),
				(unsigned char *)pBuffer + uBytesRead,
				dwBytesToReadThisTime,
				&vOverlapped,
				&xAioCallback
			)){
				MCF_THROW(vApcResult.dwErrorCode, L"::ReadFileEx() 失败。");
			}
			::SleepEx(INFINITE, TRUE);

			if(vApcResult.dwErrorCode != ERROR_SUCCESS){
				if(vApcResult.dwErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				MCF_THROW(vApcResult.dwErrorCode, L"::ReadFileEx() 失败。");
			}
			uBytesRead += vApcResult.dwBytesTransferred;
		}
		return uBytesRead;
	}
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite, AsyncProc *pfnAsyncProc){
		DWORD dwBytesToWriteThisTime = std::min<std::size_t>(0xFFFFF000u, uBytesToWrite);

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
			MCF_THROW(vApcResult.dwErrorCode, L"::WriteFileEx() 失败。");
		}

		std::size_t uBytesWritten = vApcResult.dwBytesTransferred;
		while((uBytesWritten < uBytesToWrite) && (vApcResult.dwBytesTransferred == dwBytesToWriteThisTime)){
			dwBytesToWriteThisTime = std::min<std::size_t>(0xFFFFF000u, uBytesToWrite - uBytesWritten);

			BZero(vOverlapped);
			const auto u64NewOffset = u64Offset + uBytesWritten;
			vOverlapped.Offset = (DWORD)u64NewOffset;
			vOverlapped.OffsetHigh = (DWORD)(u64NewOffset >> 32);
			vOverlapped.hEvent = (HANDLE)&vApcResult;
			if(!::WriteFileEx(
				xm_hFile.Get(),
				(const unsigned char *)pBuffer + uBytesWritten,
				dwBytesToWriteThisTime,
				&vOverlapped,
				&xAioCallback
			)){
				MCF_THROW(vApcResult.dwErrorCode, L"::WriteFileEx() 失败。");
			}
			::SleepEx(INFINITE, TRUE);

			if(vApcResult.dwErrorCode != ERROR_SUCCESS){
				MCF_THROW(vApcResult.dwErrorCode, L"::WriteFileEx() 失败。");
			}
			uBytesWritten += vApcResult.dwBytesTransferred;
		}
	}
};

}

// 静态成员函数。
std::unique_ptr<File> File::Open(const WideStringObserver &wsoPath, bool bToRead, bool bToWrite, bool bAutoCreate){
	return std::make_unique<FileDelegate>(wsoPath, bToRead, bToWrite, bAutoCreate);
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

	return ((const FileDelegate *)this)->Read(pBuffer, uBytesToRead, u64Offset, nullptr);
}
std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset, File::AsyncProc fnAsyncProc) const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	return ((const FileDelegate *)this)->Read(pBuffer, uBytesToRead, u64Offset, &fnAsyncProc);
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite){
	ASSERT(dynamic_cast<FileDelegate *>(this));

	((FileDelegate *)this)->Write(u64Offset, pBuffer, uBytesToWrite, nullptr);
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite, File::AsyncProc fnAsyncProc){
	ASSERT(dynamic_cast<FileDelegate *>(this));

	((FileDelegate *)this)->Write(u64Offset, pBuffer, uBytesToWrite, &fnAsyncProc);
}
