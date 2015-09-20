// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "File.hpp"
#include "../Core/Exception.hpp"
#include "../Utilities/BinaryOperations.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

void *File::X_FileCloser::operator()() const noexcept {
	return INVALID_HANDLE_VALUE;
}
void File::X_FileCloser::operator()(void *hFile) const noexcept {
	::CloseHandle(hFile);
}

// 构造函数和析构函数。
File::File(const wchar_t *pwszPath, std::uint32_t u32Flags){
	DWORD dwDesiredAccess = 0;
	if(u32Flags & kToRead){
		dwDesiredAccess |= GENERIC_READ;
	}
	if(u32Flags & kToWrite){
		dwDesiredAccess |= GENERIC_WRITE | FILE_READ_ATTRIBUTES;
	}

	DWORD dwShareMode = FILE_SHARE_READ;
	if(u32Flags & kToWrite){
		dwShareMode = 0;
	}

	DWORD dwCreateDisposition;
	if(u32Flags & kToWrite){
		if(u32Flags & kDontCreate){
			dwCreateDisposition = OPEN_EXISTING;
		} else if(u32Flags & kFailIfExists){
			dwCreateDisposition = CREATE_NEW;
		} else {
			dwCreateDisposition = OPEN_ALWAYS;
		}
	} else {
		dwCreateDisposition = OPEN_EXISTING;
	}

	DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
	if(u32Flags & kNoBuffering){
		dwFlagsAndAttributes |= FILE_FLAG_NO_BUFFERING;
	}
	if(u32Flags & kWriteThrough){
		dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
	}
	if(u32Flags & kDeleteOnClose){
		dwFlagsAndAttributes |= FILE_FLAG_DELETE_ON_CLOSE;
	}

	if(!x_hFile.Reset(::CreateFileW(pwszPath, dwDesiredAccess, dwShareMode, nullptr, dwCreateDisposition, dwFlagsAndAttributes, NULL))){
		DEBUG_THROW(SystemError, "CreateFileW"_rcs);
	}

	if((u32Flags & kToWrite) && !(u32Flags & kDontTruncate)){
		if(!::SetEndOfFile(x_hFile.Get())){
			DEBUG_THROW(SystemError, "SetEndOfFile"_rcs);
		}
		if(!::FlushFileBuffers(x_hFile.Get())){
			DEBUG_THROW(SystemError, "FlushFileBuffers"_rcs);
		}
	}
}
File::File(const WideString &wsPath, std::uint32_t u32Flags)
	: File(wsPath.GetStr(), u32Flags)
{
}
File::~File(){
}

// 其他非静态成员函数。
bool File::IsOpen() const noexcept {
	return !!x_hFile;
}
void File::Open(const wchar_t *pwszPath, std::uint32_t u32Flags){
	File(pwszPath, u32Flags).Swap(*this);
}
void File::Open(const WideString &wsPath, std::uint32_t u32Flags){
	File(wsPath, u32Flags).Swap(*this);
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
	x_hFile.Reset();
}

std::uint64_t File::GetSize() const {
	if(!x_hFile){
		return 0;
	}

	::LARGE_INTEGER liFileSize;
	if(!::GetFileSizeEx(x_hFile.Get(), &liFileSize)){
		DEBUG_THROW(SystemError, "GetFileSizeEx"_rcs);
	}
	return (std::uint64_t)liFileSize.QuadPart;
}
void File::Resize(std::uint64_t u64NewSize){
	if(!x_hFile){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No file opened"_rcs);
	}

	::LARGE_INTEGER liNewSize;
	liNewSize.QuadPart = static_cast<long long>(u64NewSize);
	if(!::SetFilePointerEx(x_hFile.Get(), liNewSize, nullptr, FILE_BEGIN)){
		DEBUG_THROW(SystemError, "SetFilePointerEx"_rcs);
	}
	if(!::SetEndOfFile(x_hFile.Get())){
		DEBUG_THROW(SystemError, "SetEndOfFile"_rcs);
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

	DWORD dwErrorCode;
	DWORD dwTransferred;

	::OVERLAPPED vOverlapped;
	BZero(vOverlapped);
	vOverlapped.Offset = u64Offset;
	vOverlapped.OffsetHigh = (u64Offset >> 32);
	if(::ReadFile(x_hFile.Get(), pBuffer, u32BytesToRead, nullptr, &vOverlapped)){
		dwErrorCode = ERROR_SUCCESS;
	} else {
		dwErrorCode = ::GetLastError();
	}
	if(fnAsyncProc){
		fnAsyncProc();
	}
	if(dwErrorCode != ERROR_SUCCESS){
		if(dwErrorCode != ERROR_IO_PENDING){
			DEBUG_THROW(SystemError, dwErrorCode, "ReadFile"_rcs);
		}
		if(!::GetOverlappedResult(x_hFile.Get(), &vOverlapped, &dwTransferred, true)){
			if(::GetLastError() != ERROR_HANDLE_EOF){
				DEBUG_THROW(SystemError, "GetOverlappedResult"_rcs);
			}
			dwTransferred = 0;
		}
	}

	if(fnCompleteCallback){
		fnCompleteCallback();
	}
	return dwTransferred;
}
std::size_t File::Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite,
	FunctionObserver<void ()> fnAsyncProc, FunctionObserver<void ()> fnCompleteCallback)
{
	if(!x_hFile){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No file opened"_rcs);
	}

	DWORD dwErrorCode;
	DWORD dwTransferred;

	::OVERLAPPED vOverlapped;
	BZero(vOverlapped);
	vOverlapped.Offset = u64Offset;
	vOverlapped.OffsetHigh = (u64Offset >> 32);
	if(::WriteFile(x_hFile.Get(), pBuffer, u32BytesToWrite, nullptr, &vOverlapped)){
		dwErrorCode = ERROR_SUCCESS;
	} else {
		dwErrorCode = ::GetLastError();
	}
	if(fnAsyncProc){
		fnAsyncProc();
	}
	if(dwErrorCode != ERROR_SUCCESS){
		if(dwErrorCode != ERROR_IO_PENDING){
			DEBUG_THROW(SystemError, dwErrorCode, "WriteFile"_rcs);
		}
		if(!::GetOverlappedResult(x_hFile.Get(), &vOverlapped, &dwTransferred, true)){
			DEBUG_THROW(SystemError, "GetOverlappedResult"_rcs);
		}
	}

	if(fnCompleteCallback){
		fnCompleteCallback();
	}
	return dwTransferred;
}
void File::Flush() const {
	if(!x_hFile){
		DEBUG_THROW(Exception, ERROR_INVALID_HANDLE, "No file opened"_rcs);
	}

	if(!::FlushFileBuffers(x_hFile.Get())){
		DEBUG_THROW(SystemError, "FlushFileBuffers"_rcs);
	}
}

}
