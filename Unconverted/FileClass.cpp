// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "FileClass.hpp"
#include "xFileThreadClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
FileClass::FileClass(bool bAsyncMode){
	xInit();
	xm_pFileThread = new xFileThreadClass(this, bAsyncMode);
}
FileClass::~FileClass(){
	Close();
	delete xm_pFileThread;
}

// 其他非静态成员函数。
void FileClass::xInit(){
	xm_hFile				= INVALID_HANDLE_VALUE;
	xm_bModeRead			= false;
	xm_bModeWrite			= false;
	xm_ullFilePointerRead	= 0;
	xm_ullFilePointerWrite	= 0;

	xm_dwErrorCode = ERROR_SUCCESS;
}

void FileClass::xOnReadBlock(DWORD dwBytesRead, DWORD dwBytesTotal){
	UNREF_PARAM(dwBytesRead);
	UNREF_PARAM(dwBytesTotal);
}
void FileClass::xOnWriteBlock(DWORD dwBytesWritten, DWORD dwBytesTotal){
	UNREF_PARAM(dwBytesWritten);
	UNREF_PARAM(dwBytesTotal);
}

bool FileClass::Open(LPCTSTR pszFilePath, bool bWantRead, bool bWantWrite, bool bCreateIfNotExist, bool bTruncateIfExistWhenWrite){
	Close();

	xm_hFile = ::CreateFile(
		pszFilePath,
		(DWORD)((bWantRead ? GENERIC_READ : 0) | (bWantWrite ? GENERIC_WRITE : 0)),
		(DWORD)(bWantWrite ? 0 : FILE_SHARE_READ),
		nullptr,
		(DWORD)(bCreateIfNotExist ? OPEN_ALWAYS : OPEN_EXISTING),
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	xm_dwErrorCode = ::GetLastError();

	if(xm_hFile == INVALID_HANDLE_VALUE){
		return false;
	}

	xm_bModeRead			= bWantRead;
	xm_bModeWrite			= bWantWrite;
	xm_ullFilePointerRead	= 0;
	xm_ullFilePointerWrite	= 0;

	if(bWantWrite && bTruncateIfExistWhenWrite){
		::SetEndOfFile(xm_hFile);
	}
	return true;
}
bool FileClass::OpenForReading(LPCTSTR pszFilePath){
	return Open(pszFilePath, true, false, false, false);
}
bool FileClass::OpenForWriting(LPCTSTR pszFilePath){
	return Open(pszFilePath, false, true, true, true);
}
bool FileClass::OpenForAppending(LPCTSTR pszFilePath){
	return Open(pszFilePath, false, true, true, false);
}
void FileClass::Close(){
	Wait();

	if(xm_hFile != INVALID_HANDLE_VALUE){
		::CloseHandle(xm_hFile);

		xm_hFile				= INVALID_HANDLE_VALUE;
		xm_bModeRead			= false;
		xm_bModeWrite			= false;
		xm_ullFilePointerRead	= 0;
		xm_ullFilePointerWrite	= 0;
	}
	xm_dwErrorCode = ERROR_SUCCESS;
}
bool FileClass::IsOpen() const {
	return xm_hFile != INVALID_HANDLE_VALUE;
}

DWORD FileClass::GetErrorCode() const {
	return xm_dwErrorCode;
}

bool FileClass::IsIdle() const {
	return xm_pFileThread->WaitTimeout(0);
}
void FileClass::Wait() const {
	xm_pFileThread->Wait();
}
bool FileClass::WaitTimeout(DWORD dwMilliSeconds) const {
	return xm_pFileThread->WaitTimeout(dwMilliSeconds);
}

unsigned long long FileClass::GetSize() const {
	if(xm_hFile == INVALID_HANDLE_VALUE){
		xm_dwErrorCode = ERROR_INVALID_HANDLE;
		return INVALID_SIZE;
	}

	LARGE_INTEGER liFileSize;
	if(::GetFileSizeEx(xm_hFile, &liFileSize) == FALSE){
		xm_dwErrorCode = ::GetLastError();
		return INVALID_SIZE;
	}
	xm_dwErrorCode = ERROR_SUCCESS;
	return (unsigned long long)liFileSize.QuadPart;
}
bool FileClass::Resize(unsigned long long ullNewSize){
	Wait();

	if(xm_hFile == INVALID_HANDLE_VALUE){
		xm_dwErrorCode = ERROR_INVALID_HANDLE;
		return false;
	}
	if(!xm_bModeWrite){
		xm_dwErrorCode = ERROR_NOT_SUPPORTED;
		return false;
	}

	LARGE_INTEGER liFilePos;
	liFilePos.QuadPart = (LONGLONG)ullNewSize;
	if(::SetFilePointerEx(xm_hFile, liFilePos, nullptr, FILE_BEGIN) == FALSE){
		xm_dwErrorCode = ::GetLastError();
		return false;
	}
	if(::SetEndOfFile(xm_hFile) == FALSE){
		xm_dwErrorCode = ::GetLastError();
		return false;
	}
	xm_dwErrorCode = ERROR_SUCCESS;
	return true;
}

unsigned long long FileClass::TellRead() const {
	if(!xm_bModeRead){
		xm_dwErrorCode = ERROR_NOT_SUPPORTED;
		return INVALID_SIZE;
	}
	return xm_ullFilePointerRead;
}
unsigned long long FileClass::TellWrite() const {
	if(!xm_bModeWrite){
		xm_dwErrorCode = ERROR_NOT_SUPPORTED;
		return INVALID_SIZE;
	}
	return xm_ullFilePointerWrite;
}
void FileClass::SeekRead(unsigned long long ullPos, bool bRelative){
	ASSERT(xm_bModeRead);

	Wait();

	if(bRelative){
		xm_ullFilePointerRead += ullPos;
	} else {
		if((signed long long)ullPos >= 0){
			xm_ullFilePointerRead = ullPos;
		} else {
			xm_ullFilePointerRead = GetSize() + ullPos;
		}
	}
}
void FileClass::SeekWrite(unsigned long long ullPos, bool bRelative) {
	Wait();

	if(!xm_bModeWrite){
		xm_dwErrorCode = ERROR_NOT_SUPPORTED;
		return;
	}
	if(bRelative){
		xm_ullFilePointerWrite += ullPos;
	} else {
		if((signed long long)ullPos >= 0){
			xm_ullFilePointerWrite = ullPos;
		} else {
			xm_ullFilePointerWrite = GetSize() + ullPos;
		}
	}
}

bool FileClass::Read(void *pBuffer, std::size_t uSize){
	Wait();

	if(!xm_bModeRead){
		xm_dwErrorCode = ERROR_NOT_SUPPORTED;
		return false;
	}
	if(xm_hFile == INVALID_HANDLE_VALUE){
		xm_dwErrorCode = ERROR_INVALID_HANDLE;
		return false;
	}
	xm_pFileThread->BeginReading(pBuffer, uSize);
	return true;
}
bool FileClass::Write(const void *pBuffer, std::size_t uSize){
	Wait();

	if(!xm_bModeWrite){
		xm_dwErrorCode = ERROR_NOT_SUPPORTED;
		return false;
	}
	if(xm_hFile == INVALID_HANDLE_VALUE){
		xm_dwErrorCode = ERROR_INVALID_HANDLE;
		return false;
	}
	xm_pFileThread->BeginWriting(pBuffer, uSize);
	return true;
}
void FileClass::Cancel(){
	ASSERT(xm_bModeRead || xm_bModeWrite);

	xm_pFileThread->Cancel();
}
