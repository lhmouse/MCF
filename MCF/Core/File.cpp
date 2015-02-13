// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "File.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
#include "../Utilities/BinaryOperations.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

void *File::xFileCloser::operator()() const noexcept {
	return INVALID_HANDLE_VALUE;
}
void File::xFileCloser::operator()(void *hFile) const noexcept {
	::CloseHandle(hFile);
}

// 构造函数和析构函数。
File::File(const wchar_t *pwszPath, std::uint32_t u32Flags){
	Open(pwszPath, u32Flags);
}
File::File(const WideString &wsPath, std::uint32_t u32Flags){
	Open(wsPath, u32Flags);
}

// 其他非静态成员函数。
bool File::IsOpen() const noexcept {
	return !!xm_hFile;
}
void File::Open(const wchar_t *pwszPath, std::uint32_t u32Flags){
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

	if(!xm_hFile.Reset(::CreateFileW(pwszPath,
		((u32Flags & TO_READ) ? GENERIC_READ : 0) |
			((u32Flags & TO_WRITE) ? (GENERIC_WRITE | FILE_READ_ATTRIBUTES) : 0),
		(u32Flags & TO_WRITE) ? 0 : FILE_SHARE_READ,
		nullptr, dwCreateDisposition, dwFlagsAndAttributes, NULL)))
	{
		DEBUG_THROW(SystemError, "CreateFileW");
	}
	if((u32Flags & TO_WRITE) && !(u32Flags & NO_TRUNC)){
		Resize(0);
		Flush();
	}
}
void File::Open(const WideString &wsPath, std::uint32_t u32Flags){
	Open(wsPath.GetStr(), u32Flags);
}
bool File::OpenNoThrow(const wchar_t *pwszPath, std::uint32_t u32Flags){
	try {
		Open(pwszPath, u32Flags);
		return true;
	} catch(SystemError &e){
		::SetLastError(e.GetCode());
		return false;
	}
}
bool File::OpenNoThrow(const WideString &wsPath, std::uint32_t u32Flags){
	try {
		Open(wsPath, u32Flags);
		return true;
	} catch(SystemError &e){
		::SetLastError(e.GetCode());
		return false;
	}
}
void File::Close() noexcept {
	xm_hFile.Reset();
}

std::uint64_t File::GetSize() const {
	if(!xm_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
	}

	::LARGE_INTEGER liFileSize;
	if(!::GetFileSizeEx(xm_hFile.Get(), &liFileSize)){
		DEBUG_THROW(SystemError, "GetFileSizeEx");
	}
	return (std::uint64_t)liFileSize.QuadPart;
}
void File::Resize(std::uint64_t u64NewSize){
	if(!xm_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
	}

	::LARGE_INTEGER liNewSize;
	liNewSize.QuadPart = (long long)u64NewSize;
	if(!::SetFilePointerEx(xm_hFile.Get(), liNewSize, nullptr, FILE_BEGIN)){
		DEBUG_THROW(SystemError, "SetFilePointerEx");
	}
	if(!::SetEndOfFile(xm_hFile.Get())){
		DEBUG_THROW(SystemError, "SetEndOfFile");
	}
}
void File::Clear(){
	Resize(0);
}

std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset,
	const std::function<void ()> &fnAsyncProc, const std::function<void ()> &fnCompleteCallback) const
{
	if(!xm_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
	}

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
	if(fnAsyncProc){
		fnAsyncProc();
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

	if(fnCompleteCallback){
		fnCompleteCallback();
	}
	return uBytesRead;
}
void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite,
	const std::function<void ()> &fnAsyncProc, const std::function<void ()> &fnCompleteCallback)
{
	if(!xm_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
	}

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
	if(fnAsyncProc){
		fnAsyncProc();
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

	if(fnCompleteCallback){
		fnCompleteCallback();
	}
}
void File::Flush() const {
	if(!xm_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
	}

	if(!::FlushFileBuffers(xm_hFile.Get())){
		DEBUG_THROW(SystemError, "FlushFileBuffers");
	}
}

}
