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

	struct xApcResult {
		std::uint32_t u32BytesTransferred;
		DWORD dwErrorCode;
	};

private:
	static void __stdcall xAIOCallback(DWORD dwErrorCode, DWORD dwBytesTransferred, LPOVERLAPPED pOverlapped) noexcept {
		const auto pApcResult = (xApcResult *)pOverlapped->hEvent;
		pApcResult->dwErrorCode = dwErrorCode;
		pApcResult->u32BytesTransferred = dwBytesTransferred;
	}

private:
	UniqueHandle<xFileCloser> xm_hFile;

public:
	~xDelegate(){
		Close();
	}

public:
	bool IsOpen() const noexcept {
		return xm_hFile.IsGood();
	}
	void Open(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate){
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
			MCF_THROW(::GetLastError(), L"::CreateFileW() 失败。");
		}
	}
	void Close() noexcept {
		if(xm_hFile){
			xm_hFile.Reset();
		}
	}

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

	std::uint32_t Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead, AsyncProc *pfnAsyncProc) const {
		xApcResult ApcResult;
		ApcResult.u32BytesTransferred = 0;
		ApcResult.dwErrorCode = ERROR_SUCCESS;

		OVERLAPPED Overlapped;
		BZero(Overlapped);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);
		Overlapped.hEvent = (HANDLE)&ApcResult;
		const bool bSucceeds = ::ReadFileEx(xm_hFile.Get(), pBuffer, u32BytesToRead, &Overlapped, &xAIOCallback);
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
		return ApcResult.u32BytesTransferred;
	}
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite, AsyncProc *pfnAsyncProc){
		xApcResult ApcResult;
		ApcResult.u32BytesTransferred = 0;
		ApcResult.dwErrorCode = ERROR_SUCCESS;

		OVERLAPPED Overlapped;
		BZero(Overlapped);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);
		Overlapped.hEvent = (HANDLE)&ApcResult;
		const bool bSucceeds = ::WriteFileEx(xm_hFile.Get(), pBuffer, u32BytesToWrite, &Overlapped, &xAIOCallback);
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

// 构造函数和析构函数。
File::File() noexcept {
}
File::File(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate){
	Open(pwszPath, bToRead, bToWrite, bAutoCreate);
}
File::File(File &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
File &File::operator=(File &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
File::~File(){
}

// 其他非静态成员函数。
bool File::IsOpen() const noexcept {
	if(!xm_pDelegate){
		return false;
	}
	return xm_pDelegate->IsOpen();
}
unsigned long File::OpenNoThrow(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate){
	try {
		Open(pwszPath, bToRead, bToWrite, bAutoCreate);
		return ERROR_SUCCESS;
	} catch(Exception &e){
		return e.ulErrorCode;
	}
}
void File::Open(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate){
	if(!xm_pDelegate){
		xm_pDelegate.reset(new xDelegate);
	}
	xm_pDelegate->Open(pwszPath, bToRead, bToWrite, bAutoCreate);
}
void File::Close() noexcept {
	if(!xm_pDelegate){
		return;
	}
	xm_pDelegate->Close();
}

std::uint64_t File::GetSize() const {
	if(!xm_pDelegate){
		MCF_THROW(ERROR_INVALID_HANDLE, L"没有打开文件。");
	}
	return xm_pDelegate->GetSize();
}
void File::Resize(std::uint64_t u64NewSize){
	if(!xm_pDelegate){
		MCF_THROW(ERROR_INVALID_HANDLE, L"没有打开文件。");
	}
	return xm_pDelegate->Resize(u64NewSize);
}

std::uint32_t File::Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead) const {
	if(!xm_pDelegate){
		MCF_THROW(ERROR_INVALID_HANDLE, L"没有打开文件。");
	}
	return xm_pDelegate->Read(pBuffer, u64Offset, u32BytesToRead, nullptr);
}
std::uint32_t File::Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead, File::AsyncProc fnAsyncProc) const {
	if(!xm_pDelegate){
		MCF_THROW(ERROR_INVALID_HANDLE, L"没有打开文件。");
	}
	return xm_pDelegate->Read(pBuffer, u64Offset, u32BytesToRead, &fnAsyncProc);
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite){
	if(!xm_pDelegate){
		MCF_THROW(ERROR_INVALID_HANDLE, L"没有打开文件。");
	}
	xm_pDelegate->Write(u64Offset, pBuffer, u32BytesToWrite, nullptr);
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite, File::AsyncProc fnAsyncProc){
	if(!xm_pDelegate){
		MCF_THROW(ERROR_INVALID_HANDLE, L"没有打开文件。");
	}
	xm_pDelegate->Write(u64Offset, pBuffer, u32BytesToWrite, &fnAsyncProc);
}
