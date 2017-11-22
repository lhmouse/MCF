// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "file.h"
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

bool MCFBUILD_FileGetContents(void *restrict *restrict ppData, MCFBUILD_STD size_t *puSize, const wchar_t *restrict pwcPath){
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
		CloseHandle(hFile);
		SetLastError(dwErrorCode);
		return false;
	}
	// Reject overlarge files.
	if(liFileSize.QuadPart > PTRDIFF_MAX){
		CloseHandle(hFile);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	size_t uSize = (size_t)liFileSize.QuadPart;
	// Allocate the buffer that is to be freed using `MCFBUILD_FileFreeContents()`.
	void *pData = HeapAlloc(GetProcessHeap(), 0, uSize);
	if(!pData){
		CloseHandle(hFile);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	// Read data in a loop, up to the specified number of bytes.
	size_t uBytesTotal = 0;
	for(;;){
		DWORD dwBytesToRead = SubtractWithSaturation(uSize, uBytesTotal);
		if(dwBytesToRead == 0){
			break;
		}
		DWORD dwBytesRead;
		if(!ReadFile(hFile, (char *)pData + uBytesTotal, dwBytesToRead, &dwBytesRead, 0)){
			// If an error occurs, deallocate the buffer and bail out.
			dwErrorCode = GetLastError();
			HeapFree(GetProcessHeap(), 0, pData);
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		if(dwBytesRead == 0){
			// EOF encountered. Stop.
			break;
		}
		uBytesTotal += dwBytesRead;
	}
	CloseHandle(hFile);
	// Return this buffer to the caller.
	*ppData = pData;
	*puSize = uBytesTotal;
	return true;
}
void MCFBUILD_FileFreeContents(void *pData){
	// Be warned that passing a null pointer to `HeapFree()` is undefined behavior.
	if(pData){
		HeapFree(GetProcessHeap(), 0, pData);
	}
}
bool MCFBUILD_FileGetSha256(uint8_t (*restrict pau8Result)[32], const wchar_t *restrict pwcPath){
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
		DWORD dwBytesRead;
		if(!ReadFile(hFile, abyTemp, sizeof(abyTemp), &dwBytesRead, 0)){
			// If an error occurs, bail out.
			dwErrorCode = GetLastError();
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		if(dwBytesRead == 0){
			// EOF encountered. Stop.
			break;
		}
		MCFBUILD_Sha256Update(&vContext, abyTemp, dwBytesRead);
	}
	CloseHandle(hFile);
	// Write the result.
	MCFBUILD_Sha256Finalize(pau8Result, &vContext);
	return true;
}

bool MCFBUILD_FilePutContents(const wchar_t *pwcPath, const void *pData, size_t uSize){
	DWORD dwErrorCode;
	// Open the file for writing. Create one if it does not exist. Any existent data are discarded.
	HANDLE hFile = CreateFileW(pwcPath, FILE_WRITE_DATA, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	// Write data in a loop, up to the specified number of bytes.
	size_t uBytesTotal = 0;
	for(;;){
		DWORD dwBytesToWrite = SubtractWithSaturation(uSize, uBytesTotal);
		if(dwBytesToWrite == 0){
			break;
		}
		DWORD dwBytesWritten;
		if(!WriteFile(hFile, (const char *)pData + uBytesTotal, dwBytesToWrite, &dwBytesWritten, 0)){
			// If an error occurs, bail out.
			dwErrorCode = GetLastError();
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		uBytesTotal += dwBytesWritten;
	}
	CloseHandle(hFile);
	return true;
}
bool MCFBUILD_FileAppendContents(const wchar_t *pwcPath, const void *pData, size_t uSize){
	DWORD dwErrorCode;
	// Open the file for appending. Create one if it does not exist. Any existent data are left alone.
	HANDLE hFile = CreateFileW(pwcPath, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	// Set the file pointer to the EOF.
	LARGE_INTEGER liFilePointer;
	liFilePointer.QuadPart = 0;
	if(!SetFilePointerEx(hFile, liFilePointer, 0, FILE_END)){
		dwErrorCode = GetLastError();
		CloseHandle(hFile);
		SetLastError(dwErrorCode);
		return false;
	}
	// Write data in a loop, up to the specified number of bytes.
	size_t uBytesTotal = 0;
	for(;;){
		DWORD dwBytesToWrite = SubtractWithSaturation(uSize, uBytesTotal);
		if(dwBytesToWrite == 0){
			break;
		}
		DWORD dwBytesWritten;
		if(!WriteFile(hFile, (const char *)pData + uBytesTotal, dwBytesToWrite, &dwBytesWritten, 0)){
			// If an error occurs, bail out.
			dwErrorCode = GetLastError();
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		uBytesTotal += dwBytesWritten;
	}
	CloseHandle(hFile);
	return true;
}
