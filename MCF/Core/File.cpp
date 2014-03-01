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

// 嵌套类定义。
class File::xDelegate : NO_COPY {
private:
	struct xFileCloser {
		constexpr HANDLE operator()() const {
			return INVALID_HANDLE_VALUE;
		}
		void operator()(HANDLE hFile) const {
			::CloseHandle(hFile);
		}
	};

	typedef struct xtagApcResult {
		std::uint32_t u32BytesTransferred;
		DWORD dwErrorCode;
	} xAPC_RESULT;
private:
	static void __stdcall xAIOCallback(DWORD dwErrorCode, DWORD dwBytesTransferred, LPOVERLAPPED pOverlapped) noexcept {
		const auto pApcResult = (xAPC_RESULT *)pOverlapped->hEvent;
		pApcResult->dwErrorCode = dwErrorCode;
		pApcResult->u32BytesTransferred = dwBytesTransferred;
	}
private:
	UniqueHandle<HANDLE, xFileCloser> xm_hFile;
public:
	~xDelegate(){
		Close();
	}
public:
	bool IsOpen() const noexcept {
		return xm_hFile.IsGood();
	}
	bool Open(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate) noexcept {
		xm_hFile.Reset(::CreateFileW(
			pwszPath,
			(bToRead ? GENERIC_READ : 0) | (bToWrite ? GENERIC_WRITE : 0),
			bToWrite ? 0 : FILE_SHARE_READ,
			nullptr,
			(bToWrite && bAutoCreate) ? OPEN_ALWAYS : OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL
		));
		if(!xm_hFile){
			return false;
		}
		return true;
	}
	void Close() noexcept {
		if(xm_hFile){
			xm_hFile.Reset();
		}
	}

	std::uint64_t GetSize() const noexcept {
		LARGE_INTEGER liFileSize;
		if(!::GetFileSizeEx(xm_hFile, &liFileSize)){
			return INVALID_SIZE;
		}
		return (std::uint64_t)liFileSize.QuadPart;
	}
	bool Resize(std::uint64_t u64NewSize) noexcept {
		if(u64NewSize > (std::uint64_t)LLONG_MAX){
			::SetLastError(ERROR_INVALID_PARAMETER);
			return false;
		}
		LARGE_INTEGER liNewSize;
		liNewSize.QuadPart = (long long)u64NewSize;
		if(!::SetFilePointerEx(xm_hFile, liNewSize, nullptr, FILE_BEGIN)){
			return false;
		}
		if(!::SetEndOfFile(xm_hFile)){
			return false;
		}
		return true;
	}

	std::uint32_t Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead, ASYNC_PROC *pfnAsyncProc) const {
		xAPC_RESULT ApcResult;
		ApcResult.u32BytesTransferred = 0;
		ApcResult.dwErrorCode = ERROR_SUCCESS;

		OVERLAPPED Overlapped;
		Zero(Overlapped);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);
		Overlapped.hEvent = (HANDLE)&ApcResult;
		const bool bSucceeds = ::ReadFileEx(xm_hFile, pBuffer, u32BytesToRead, &Overlapped, &xAIOCallback);
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
			::SetLastError(ApcResult.dwErrorCode);
		}
		return ApcResult.u32BytesTransferred;
	}
	std::uint32_t Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite, ASYNC_PROC *pfnAsyncProc){
		xAPC_RESULT ApcResult;
		ApcResult.u32BytesTransferred = 0;
		ApcResult.dwErrorCode = ERROR_SUCCESS;

		OVERLAPPED Overlapped;
		Zero(Overlapped);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);
		Overlapped.hEvent = (HANDLE)&ApcResult;
		const bool bSucceeds = ::WriteFileEx(xm_hFile, pBuffer, u32BytesToWrite, &Overlapped, &xAIOCallback);
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
			::SetLastError(ApcResult.dwErrorCode);
		}
		return ApcResult.u32BytesTransferred;
	}
};

// 构造函数和析构函数。
File::File() noexcept {
}
File::File(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate)
	: File()
{
	Open(pwszPath, bToRead, bToWrite, bAutoCreate);
}
File::~File(){
	Close();
}

// 其他非静态成员函数。
bool File::IsOpen() const noexcept {
	if(!xm_pDelegate){
		return false;
	}
	return xm_pDelegate->IsOpen();
}
bool File::Open(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate){
	if(!xm_pDelegate){
		xm_pDelegate.reset(new xDelegate);
	}
	return xm_pDelegate->Open(pwszPath, bToRead, bToWrite, bAutoCreate);
}
void File::Close() noexcept {
	if(!xm_pDelegate){
		return;
	}
	xm_pDelegate->Close();
}

std::uint64_t File::GetSize() const noexcept {
	if(!xm_pDelegate){
		return INVALID_SIZE;
	}
	return xm_pDelegate->GetSize();
}
bool File::Resize(std::uint64_t u64NewSize) noexcept {
	if(!xm_pDelegate){
		return false;
	}
	return xm_pDelegate->Resize(u64NewSize);
}

std::uint32_t File::Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead) const noexcept {
	if(!xm_pDelegate){
		::SetLastError(ERROR_INVALID_HANDLE);
		return 0;
	}
	return xm_pDelegate->Read(pBuffer, u64Offset, u32BytesToRead, nullptr);
}
std::uint32_t File::Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead, File::ASYNC_PROC fnAsyncProc) const {
	if(!xm_pDelegate){
		::SetLastError(ERROR_INVALID_HANDLE);
		return 0;
	}
	return xm_pDelegate->Read(pBuffer, u64Offset, u32BytesToRead, &fnAsyncProc);
}
std::uint32_t File::Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite) noexcept {
	if(!xm_pDelegate){
		::SetLastError(ERROR_INVALID_HANDLE);
		return 0;
	}
	return xm_pDelegate->Write(u64Offset, pBuffer, u32BytesToWrite, nullptr);
}
std::uint32_t File::Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite, File::ASYNC_PROC fnAsyncProc){
	if(!xm_pDelegate){
		::SetLastError(ERROR_INVALID_HANDLE);
		return 0;
	}
	return xm_pDelegate->Write(u64Offset, pBuffer, u32BytesToWrite, &fnAsyncProc);
}
