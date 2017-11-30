// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN 1
#include "file.h"
#include "heap.h"
#include "last_error.h"
#include "sha256.h"
#include <windows.h>

static inline DWORD SubtractWithSaturation(size_t uMinuend, size_t uSubtrahend){
	size_t uDifference;
	if(uMinuend <= uSubtrahend){
		uDifference = 0;
	} else if((uDifference = uMinuend - uSubtrahend) > UINT32_MAX){
		uDifference = UINT32_MAX;
	}
	return (DWORD)uDifference;
}

static void CheckedCloseHandle(HANDLE hObject){
	if(!CloseHandle(hObject)){
		OutputDebugStringA("*** FATAL *** Failed to close handle of kernel object.");
		DebugBreak();
	}
}

bool MCFBUILD_FileGetContents(void **restrict ppData, MCFBUILD_STD size_t *restrict puSize, const wchar_t *pwcPath){
	DWORD dwErrorCode;
	// Open the file for reading. Fail if it does not exist.
	HANDLE hFile = CreateFileW(pwcPath, FILE_READ_DATA, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	// Query the number of bytes in it.
	LARGE_INTEGER liFileSize;
	if(!GetFileSizeEx(hFile, &liFileSize)){
		dwErrorCode = GetLastError();
		CheckedCloseHandle(hFile);
		MCFBUILD_SetLastError(dwErrorCode);
		return false;
	}
	// Reject overlarge files.
	if(liFileSize.QuadPart > PTRDIFF_MAX - 4){
		CheckedCloseHandle(hFile);
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	size_t uCapacity = (size_t)liFileSize.QuadPart;
	// Allocate the buffer that is to be freed using `MCFBUILD_FileFreeContentBuffer()`.
	unsigned char *pbyData = MCFBUILD_HeapAlloc(uCapacity + 4);
	if(!pbyData){
		CheckedCloseHandle(hFile);
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	// Read data in a loop, up to the specified number of bytes.
	size_t uSizeTotal = 0;
	for(;;){
		DWORD dwSizeToRead = SubtractWithSaturation(uCapacity, uSizeTotal);
		if(dwSizeToRead == 0){
			break;
		}
		DWORD dwSizeRead;
		if(!ReadFile(hFile, pbyData + uSizeTotal, dwSizeToRead, &dwSizeRead, 0)){
			// If an error occurs, deallocate the buffer and bail out.
			dwErrorCode = GetLastError();
			MCFBUILD_HeapFree(pbyData);
			CheckedCloseHandle(hFile);
			MCFBUILD_SetLastError(dwErrorCode);
			return false;
		}
		if(dwSizeRead == 0){
			// EOF encountered. Stop.
			break;
		}
		uSizeTotal += dwSizeRead;
	}
	// Terminate contents with four zero bytes.
	memset(pbyData + uSizeTotal, 0, 4);
	CheckedCloseHandle(hFile);
	// Hand over the buffer to our caller.
	*ppData = pbyData;
	*puSize = uSizeTotal;
	return true;
}
void MCFBUILD_FileFreeContentBuffer(void *pData){
	MCFBUILD_HeapFree(pData);
}
bool MCFBUILD_FileGetSha256(MCFBUILD_Sha256 *pau8Sha256, const wchar_t *pwcPath){
	DWORD dwErrorCode;
	// Open the file for reading. Fail if it does not exist.
	HANDLE hFile = CreateFileW(pwcPath, FILE_READ_DATA, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	// Read data in a loop, until no more bytes are to be read.
	MCFBUILD_Sha256Context vContext;
	MCFBUILD_Sha256Initialize(&vContext);
	for(;;){
		unsigned char abyTemp[4096];
		DWORD dwSizeRead;
		if(!ReadFile(hFile, abyTemp, sizeof(abyTemp), &dwSizeRead, 0)){
			// If an error occurs, bail out.
			dwErrorCode = GetLastError();
			CheckedCloseHandle(hFile);
			MCFBUILD_SetLastError(dwErrorCode);
			return false;
		}
		if(dwSizeRead == 0){
			// EOF encountered. Stop.
			break;
		}
		MCFBUILD_Sha256Update(&vContext, abyTemp, dwSizeRead);
	}
	CheckedCloseHandle(hFile);
	// Write the result.
	MCFBUILD_Sha256Finalize(pau8Sha256, &vContext);
	return true;
}

bool MCFBUILD_FilePutContents(const wchar_t *pwcPath, const void *pData, size_t uSize){
	DWORD dwErrorCode;
	// Open the file for writing. Create one if it does not exist. Any existent data are discarded.
	HANDLE hFile = CreateFileW(pwcPath, FILE_WRITE_DATA, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	const unsigned char *pbyData = pData;
	// Write data in a loop, up to the specified number of bytes.
	size_t uSizeTotal = 0;
	for(;;){
		DWORD dwSizeToWrite = SubtractWithSaturation(uSize, uSizeTotal);
		if(dwSizeToWrite == 0){
			break;
		}
		DWORD dwSizeWritten;
		if(!WriteFile(hFile, pbyData + uSizeTotal, dwSizeToWrite, &dwSizeWritten, 0)){
			// If an error occurs, bail out.
			dwErrorCode = GetLastError();
			CheckedCloseHandle(hFile);
			MCFBUILD_SetLastError(dwErrorCode);
			return false;
		}
		uSizeTotal += dwSizeWritten;
	}
	CheckedCloseHandle(hFile);
	return true;
}
bool MCFBUILD_FileAppendContents(const wchar_t *pwcPath, const void *pData, size_t uSize){
	DWORD dwErrorCode;
	// Open the file for appending. Create one if it does not exist. Any existent data are left alone.
	HANDLE hFile = CreateFileW(pwcPath, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	const unsigned char *pbyData = pData;
	// Write data in a loop, up to the specified number of bytes.
	size_t uSizeTotal = 0;
	for(;;){
		DWORD dwSizeToWrite = SubtractWithSaturation(uSize, uSizeTotal);
		if(dwSizeToWrite == 0){
			break;
		}
		// Override the file pointer with this offset.
		// See <https://msdn.microsoft.com/en-us/library/windows/desktop/aa365747.aspx>.
		OVERLAPPED vOverlapped = { 0 };
		vOverlapped.Offset = ULONG_MAX;
		vOverlapped.OffsetHigh = ULONG_MAX;
		DWORD dwSizeWritten;
		if(!WriteFile(hFile, pbyData + uSizeTotal, dwSizeToWrite, &dwSizeWritten, &vOverlapped)){
			// If an error occurs, bail out.
			dwErrorCode = GetLastError();
			CheckedCloseHandle(hFile);
			MCFBUILD_SetLastError(dwErrorCode);
			return false;
		}
		uSizeTotal += dwSizeWritten;
	}
	CheckedCloseHandle(hFile);
	return true;
}

bool MCFBUILD_FileLock(uintptr_t *restrict puCookie, const wchar_t *pwcPath){
	// Open the file without access to its data. Create one if it does not exist.
	HANDLE hFile = CreateFileW(pwcPath, 0, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	*puCookie = (uintptr_t)EncodePointer(hFile);
	return true;
}
void MCFBUILD_FileUnlock(uintptr_t uCookie){
	HANDLE hFile = DecodePointer((HANDLE)uCookie);
	CheckedCloseHandle(hFile);
}
