// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "file.h"
#include <windows.h>

bool MCFBUILD_FileGetContents(void **ppData, MCFBUILD_STD size_t *puSize, const wchar_t *pwcPath){
	DWORD dwErrorCode;
	HANDLE hFile = CreateFileW(pwcPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	LARGE_INTEGER ilFileSize;
	if(!GetFileSizeEx(hFile, &ilFileSize)){
		dwErrorCode = GetLastError();
		CloseHandle(hFile);
		SetLastError(dwErrorCode);
		return false;
	}
	DWORD dwBytesToReadTotal = (DWORD)ilFileSize.QuadPart;
	if((LONGLONG)dwBytesToReadTotal != ilFileSize.QuadPart){
		CloseHandle(hFile);
		SetLastError(ERROR_ARITHMETIC_OVERFLOW);
		return false;
	}
	void *pData = HeapAlloc(GetProcessHeap(), 0, dwBytesToReadTotal);
	if(!pData){
		CloseHandle(hFile);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	DWORD dwBytesReadTotal = 0;
	for(;;){
		if(dwBytesToReadTotal <= dwBytesReadTotal){
			break;
		}
		DWORD dwBytesRead;
		if(!ReadFile(hFile, (char *)pData + dwBytesReadTotal, dwBytesToReadTotal - dwBytesReadTotal, &dwBytesRead, NULL)){
			dwErrorCode = GetLastError();
			HeapFree(GetProcessHeap(), 0, pData);
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		if(dwBytesRead == 0){
			break;
		}
		dwBytesReadTotal += dwBytesRead;
	}
	CloseHandle(hFile);
	*ppData = pData;
	*puSize = dwBytesReadTotal;
	return true;
}
void MCFBUILD_FileFreeContents(void *pData){
	if(!pData){
		return;
	}
	HeapFree(GetProcessHeap(), 0, pData);
}

bool MCFBUILD_FilePutContents(const wchar_t *pwcPath, const void *pData, size_t uSize){
	DWORD dwErrorCode;
	DWORD dwBytesToWriteTotal = (DWORD)uSize;
	if(dwBytesToWriteTotal != uSize){
		SetLastError(ERROR_ARITHMETIC_OVERFLOW);
		return false;
	}
	HANDLE hFile = CreateFileW(pwcPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	DWORD dwBytesWrittenTotal = 0;
	for(;;){
		if(dwBytesToWriteTotal <= dwBytesWrittenTotal){
			break;
		}
		DWORD dwBytesWritten;
		if(!WriteFile(hFile, (const char *)pData + dwBytesWrittenTotal, dwBytesToWriteTotal - dwBytesWrittenTotal, &dwBytesWritten, NULL)){
			dwErrorCode = GetLastError();
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		dwBytesWrittenTotal += dwBytesWritten;
	}
	CloseHandle(hFile);
	return true;
}
bool MCFBUILD_FileAppendContents(const wchar_t *pwcPath, const void *pData, size_t uSize){
	DWORD dwErrorCode;
	DWORD dwBytesToWriteTotal = (DWORD)uSize;
	if(dwBytesToWriteTotal != uSize){
		SetLastError(ERROR_ARITHMETIC_OVERFLOW);
		return false;
	}
	HANDLE hFile = CreateFileW(pwcPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	LARGE_INTEGER liPosition;
	liPosition.QuadPart = 0;
	if(!SetFilePointerEx(hFile, liPosition, NULL, FILE_END)){
		dwErrorCode = GetLastError();
		CloseHandle(hFile);
		SetLastError(dwErrorCode);
		return false;
	}
	DWORD dwBytesWrittenTotal = 0;
	for(;;){
		if(dwBytesToWriteTotal <= dwBytesWrittenTotal){
			break;
		}
		DWORD dwBytesWritten;
		if(!WriteFile(hFile, (const char *)pData + dwBytesWrittenTotal, dwBytesToWriteTotal - dwBytesWrittenTotal, &dwBytesWritten, NULL)){
			dwErrorCode = GetLastError();
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		dwBytesWrittenTotal += dwBytesWritten;
	}
	CloseHandle(hFile);
	return true;
}
