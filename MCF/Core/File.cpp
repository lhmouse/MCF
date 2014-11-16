// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "File.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
#include "../Utilities/BinaryOperations.hpp"
#include "../Utilities/MinMax.hpp"
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

private:
	UniqueHandle<xFileCloser> xm_hFile;

public:
	std::pair<unsigned long, const char *> Open(const wchar_t *pwszPath, std::uint32_t u32Flags){
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

		xm_hFile.Reset(::CreateFileW(pwszPath,
			((u32Flags & TO_READ) ? GENERIC_READ : 0) |
				((u32Flags & TO_WRITE) ? (GENERIC_WRITE | FILE_READ_ATTRIBUTES) : 0),
			(u32Flags & TO_WRITE) ? 0 : FILE_SHARE_READ,
			nullptr, dwCreateDisposition, dwFlagsAndAttributes, NULL));
		if(!xm_hFile){
			return std::make_pair(::GetLastError(), "CreateFileW");
		}
		if((u32Flags & TO_WRITE) && !(u32Flags & NO_TRUNC)){
			if(!::SetEndOfFile(xm_hFile.Get())){
				return std::make_pair(::GetLastError(), "SetEndOfFile");
			}
			if(!::FlushFileBuffers(xm_hFile.Get())){
				return std::make_pair(::GetLastError(), "FlushFileBuffers");
			}
		}
		return std::make_pair((unsigned long)ERROR_SUCCESS, nullptr);
	}

	std::uint64_t GetSize() const {
		ASSERT(xm_hFile);

		::LARGE_INTEGER liFileSize;
		if(!::GetFileSizeEx(xm_hFile.Get(), &liFileSize)){
			DEBUG_THROW(SystemError, "GetFileSizeEx");
		}
		return (std::uint64_t)liFileSize.QuadPart;
	}
	void Resize(std::uint64_t u64NewSize){
		ASSERT(xm_hFile);

		::LARGE_INTEGER liNewSize;
		liNewSize.QuadPart = (long long)u64NewSize;
		if(!::SetFilePointerEx(xm_hFile.Get(), liNewSize, nullptr, FILE_BEGIN)){
			DEBUG_THROW(SystemError, "SetFilePointerEx");
		}
		if(!::SetEndOfFile(xm_hFile.Get())){
			DEBUG_THROW(SystemError, "SetEndOfFile");
		}
	}

	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset,
		const std::function<void ()> *pfnAsyncProc, const std::function<void ()> *pfnCompleteCallback) const
	{
		ASSERT(xm_hFile);

		DWORD dwErrorCode;
		DWORD dwTransferred;
		DWORD dwBlockSize = Min(0xFFFFF000u, uBytesToRead);

		::OVERLAPPED vOverlapped;
		BZero(vOverlapped);
		vOverlapped.Offset = u64Offset;
		vOverlapped.OffsetHigh = (u64Offset >> 32);
		if(::ReadFile(xm_hFile.Get(), pBuffer, dwBlockSize, nullptr, &vOverlapped)){
			dwErrorCode = ERROR_SUCCESS;
		} else {
			dwErrorCode = ::GetLastError();
		}
		if(pfnAsyncProc){
			(*pfnAsyncProc)();
		}
		if(dwErrorCode != ERROR_SUCCESS){
			if(dwErrorCode != ERROR_IO_PENDING){
				DEBUG_THROW(SystemError, "ReadFile", dwErrorCode);
			}
			if(!::GetOverlappedResult(xm_hFile.Get(), &vOverlapped, &dwTransferred, true)){
				DEBUG_THROW(SystemError, "GetOverlappedResult");
			}
		}

		std::size_t uBytesRead = dwTransferred;
		while((uBytesRead < uBytesToRead) && (dwTransferred == dwBlockSize)){
			dwBlockSize = Min(0xFFFFF000u, uBytesToRead - uBytesRead);

			BZero(vOverlapped);
			const auto u64NewOffset = u64Offset + uBytesRead;
			vOverlapped.Offset = u64NewOffset;
			vOverlapped.OffsetHigh = (u64NewOffset >> 32);
			if(!::ReadFile(xm_hFile.Get(), (char *)pBuffer + uBytesRead, dwBlockSize, nullptr, &vOverlapped)){
				dwErrorCode = ::GetLastError();
				if(dwErrorCode != ERROR_IO_PENDING){
					DEBUG_THROW(SystemError, "ReadFile", dwErrorCode);
				}
				if(!::GetOverlappedResult(xm_hFile.Get(), &vOverlapped, &dwTransferred, true)){
					DEBUG_THROW(SystemError, "GetOverlappedResult");
				}
			}
			uBytesRead += dwTransferred;
		}

		if(pfnCompleteCallback){
			(*pfnCompleteCallback)();
		}
		return uBytesRead;
	}
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite,
		const std::function<void ()> *pfnAsyncProc, const std::function<void ()> *pfnCompleteCallback)
	{
		ASSERT(xm_hFile);

		DWORD dwErrorCode;
		DWORD dwTransferred;
		DWORD dwBlockSize = Min(0xFFFFF000u, uBytesToWrite);

		::OVERLAPPED vOverlapped;
		BZero(vOverlapped);
		vOverlapped.Offset = u64Offset;
		vOverlapped.OffsetHigh = (u64Offset >> 32);
		if(::WriteFile(xm_hFile.Get(), pBuffer, dwBlockSize, nullptr, &vOverlapped)){
			dwErrorCode = ERROR_SUCCESS;
		} else {
			dwErrorCode = ::GetLastError();
		}
		if(pfnAsyncProc){
			(*pfnAsyncProc)();
		}
		if(dwErrorCode != ERROR_SUCCESS){
			if(dwErrorCode != ERROR_IO_PENDING){
				DEBUG_THROW(SystemError, "WriteFile", dwErrorCode);
			}
			if(!::GetOverlappedResult(xm_hFile.Get(), &vOverlapped, &dwTransferred, true)){
				DEBUG_THROW(SystemError, "GetOverlappedResult");
			}
		}

		std::size_t uBytesWritten = dwTransferred;
		while(uBytesWritten < uBytesToWrite){
			dwBlockSize = Min(0xFFFFF000u, uBytesToWrite - uBytesWritten);

			BZero(vOverlapped);
			const auto u64NewOffset = u64Offset + uBytesWritten;
			vOverlapped.Offset = u64NewOffset;
			vOverlapped.OffsetHigh = (u64NewOffset >> 32);
			if(!::WriteFile(xm_hFile.Get(), (char *)pBuffer + uBytesWritten, dwBlockSize, nullptr, &vOverlapped)){
				dwErrorCode = ::GetLastError();
				if(dwErrorCode != ERROR_IO_PENDING){
					DEBUG_THROW(SystemError, "WriteFile", dwErrorCode);
				}
				if(!::GetOverlappedResult(xm_hFile.Get(), &vOverlapped, &dwTransferred, true)){
					DEBUG_THROW(SystemError, "GetOverlappedResult");
				}
			}
			uBytesWritten += dwTransferred;
		}

		if(pfnCompleteCallback){
			(*pfnCompleteCallback)();
		}
	}

	void Flush() const {
		ASSERT(xm_hFile);

		if(!::FlushFileBuffers(xm_hFile.Get())){
			DEBUG_THROW(SystemError, "FlushFileBuffers");
		}
	}
};

}

// 静态成员函数。
std::unique_ptr<File> File::Open(const wchar_t *pwszPath, std::uint32_t u32Flags){
	auto pFile = std::make_unique<FileDelegate>();
	const auto vResult = pFile->Open(pwszPath, u32Flags);
	if(vResult.first != ERROR_SUCCESS){
		DEBUG_THROW(SystemError, vResult.second, vResult.first);
	}
	return std::move(pFile);
}
std::unique_ptr<File> File::Open(const WideString &wsPath, std::uint32_t u32Flags){
	return Open(wsPath.GetCStr(), u32Flags);
}

std::unique_ptr<File> File::OpenNoThrow(const wchar_t *pwszPath, std::uint32_t u32Flags){
	auto pFile = std::make_unique<FileDelegate>();
	const auto vResult = pFile->Open(pwszPath, u32Flags);
	if(vResult.first != ERROR_SUCCESS){
		::SetLastError(vResult.first);
		return nullptr;
	}
	return std::move(pFile);
}
std::unique_ptr<File> File::OpenNoThrow(const WideString &wsPath, std::uint32_t u32Flags){
	return OpenNoThrow(wsPath.GetCStr(), u32Flags);
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

std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	return ((const FileDelegate *)this)->Read(pBuffer, uBytesToRead, u64Offset, nullptr, nullptr);
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite){
	ASSERT(dynamic_cast<FileDelegate *>(this));

	static_cast<FileDelegate *>(this)->Write(u64Offset, pBuffer, uBytesToWrite, nullptr, nullptr);
}

std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset,
	const std::function<void ()> &fnAsyncProc, const std::function<void ()> &fnCompleteCallback) const
{
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	return ((const FileDelegate *)this)->Read(pBuffer, uBytesToRead, u64Offset, &fnAsyncProc, &fnCompleteCallback);
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite,
	const std::function<void ()> &fnAsyncProc, const std::function<void ()> &fnCompleteCallback)
{
	ASSERT(dynamic_cast<FileDelegate *>(this));

	static_cast<FileDelegate *>(this)->Write(u64Offset, pBuffer, uBytesToWrite, &fnAsyncProc, &fnCompleteCallback);
}

void File::Flush() const {
	ASSERT(dynamic_cast<const FileDelegate *>(this));

	static_cast<const FileDelegate *>(this)->Flush();
}
