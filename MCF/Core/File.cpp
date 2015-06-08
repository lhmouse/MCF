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
	return !!x_hFile;
}
void File::Open(const wchar_t *pwszPath, std::uint32_t u32Flags){
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

	if(!x_hFile.Reset(::CreateFileW(pwszPath,
		((u32Flags & kToRead) ? GENERIC_READ : 0) | ((u32Flags & kToWrite) ? (GENERIC_WRITE | FILE_READ_ATTRIBUTES) : 0),
		(u32Flags & kToWrite) ? 0 : FILE_SHARE_READ,
		nullptr, dwCreateDisposition, dwFlagsAndAttributes, NULL)))
	{
		DEBUG_THROW(SystemError, "CreateFileW");
	}
	if((u32Flags & kToWrite) && !(u32Flags & kDontTruncate)){
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
	x_hFile.Reset();
}

std::uint64_t File::GetSize() const {
	if(!x_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
	}

	::LARGE_INTEGER liFileSize;
	if(!::GetFileSizeEx(x_hFile.Get(), &liFileSize)){
		DEBUG_THROW(SystemError, "GetFileSizeEx");
	}
	return (std::uint64_t)liFileSize.QuadPart;
}
void File::Resize(std::uint64_t u64NewSize){
	if(!x_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
	}

	::LARGE_INTEGER liNewSize;
	liNewSize.QuadPart = (long long)u64NewSize;
	if(!::SetFilePointerEx(x_hFile.Get(), liNewSize, nullptr, FILE_BEGIN)){
		DEBUG_THROW(SystemError, "SetFilePointerEx");
	}
	if(!::SetEndOfFile(x_hFile.Get())){
		DEBUG_THROW(SystemError, "SetEndOfFile");
	}
}
void File::Clear(){
	Resize(0);
}

std::size_t File::Read(void *pBuffer, std::uint32_t u32BytesToRead, std::uint64_t u64Offset,
	const Function<void ()> &fnAsyncProc, const Function<void ()> &fnCompleteCallback) const
{
	if(!x_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
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
			DEBUG_THROW(SystemError, "ReadFile", dwErrorCode);
		}
		if(!::GetOverlappedResult(x_hFile.Get(), &vOverlapped, &dwTransferred, true)){
			DEBUG_THROW(SystemError, "GetOverlappedResult");
		}
	}

	if(fnCompleteCallback){
		fnCompleteCallback();
	}
	return dwTransferred;
}
std::size_t File::Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite,
	const Function<void ()> &fnAsyncProc, const Function<void ()> &fnCompleteCallback)
{
	if(!x_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
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
			DEBUG_THROW(SystemError, "WriteFile", dwErrorCode);
		}
		if(!::GetOverlappedResult(x_hFile.Get(), &vOverlapped, &dwTransferred, true)){
			DEBUG_THROW(SystemError, "GetOverlappedResult");
		}
	}

	if(fnCompleteCallback){
		fnCompleteCallback();
	}
	return dwTransferred;
}
void File::Flush() const {
	if(!x_hFile){
		DEBUG_THROW(Exception, "No file opened", ERROR_INVALID_HANDLE);
	}

	if(!::FlushFileBuffers(x_hFile.Get())){
		DEBUG_THROW(SystemError, "FlushFileBuffers");
	}
}

}
