// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "xFileThreadClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
FileClass::xFileThreadClass::xFileThreadClass(FileClass *pFile, bool bAsyncMode) :
	xm_pFile		(pFile),
	xm_bAsyncMode	(bAsyncMode)
{
}
FileClass::xFileThreadClass::~xFileThreadClass(){
}

// 其他非静态成员函数。
int FileClass::xFileThreadClass::xThreadProc(){
	::SetThreadPriority(GetHThread(), THREAD_PRIORITY_BELOW_NORMAL);

	try {
		if(xm_bToRead){
			LARGE_INTEGER liPos;
			liPos.QuadPart = (LONGLONG)xm_pFile->xm_ullFilePointerRead;
			if(::SetFilePointerEx(xm_pFile->xm_hFile, liPos, nullptr, FILE_BEGIN) == FALSE){
				throw ::GetLastError();
			}

			std::size_t uBytesTotal = 0;
			while(uBytesTotal < xm_uSize){
				DWORD dwBytesRead;
				if(::ReadFile(xm_pFile->xm_hFile, (BYTE *)xm_pBuffer + (std::ptrdiff_t)uBytesTotal, std::min(xm_uSize - uBytesTotal, xBLOCK_SIZE), &dwBytesRead, nullptr) == FALSE){
					throw ::GetLastError();
				}
				if(dwBytesRead == 0){
					break;
				}

				uBytesTotal += dwBytesRead;
				xm_pFile->xm_ullFilePointerRead += dwBytesRead;

				xm_pFile->xOnReadBlock(dwBytesRead, uBytesTotal);

				if(xm_bCancelled){
					throw (DWORD)ERROR_CANCELLED;
				}
			}
		} else {
			LARGE_INTEGER liPos;
			liPos.QuadPart = (LONGLONG)xm_pFile->xm_ullFilePointerWrite;
			if(::SetFilePointerEx(xm_pFile->xm_hFile, liPos, nullptr, FILE_BEGIN) == FALSE){
				throw ::GetLastError();
			}

			std::size_t uBytesTotal = 0;
			while(uBytesTotal < xm_uSize){
				DWORD dwBytesWritten;
				if(::WriteFile(xm_pFile->xm_hFile, (BYTE *)xm_pBuffer + (std::ptrdiff_t)uBytesTotal, std::min(xm_uSize - uBytesTotal, xBLOCK_SIZE), &dwBytesWritten, nullptr) == FALSE){
					throw ::GetLastError();
				}
				if(dwBytesWritten == 0){
					break;
				}

				uBytesTotal += dwBytesWritten;
				xm_pFile->xm_ullFilePointerWrite += uBytesTotal;

				xm_pFile->xOnWriteBlock(dwBytesWritten, uBytesTotal);

				if(xm_bCancelled){
					throw (DWORD)ERROR_CANCELLED;
				}
			}
		}

		xm_pFile->xm_dwErrorCode = ERROR_SUCCESS;
	} catch(DWORD dwErrorCode){
		xm_pFile->xm_dwErrorCode = dwErrorCode;
	}
	return 0;
}

void FileClass::xFileThreadClass::BeginReading(void *pBuffer, std::size_t uSize){
	xm_bToRead	= true;
	xm_pBuffer	= pBuffer;
	xm_uSize	= uSize;

	xm_bCancelled = false;

	if(xm_bAsyncMode){
		Create(false);
	} else {
		ParasitizeCurrentThread(false, false);
	}
}
void FileClass::xFileThreadClass::BeginWriting(const void *pBuffer,  std::size_t uSize){
	xm_bToRead	= false;
	xm_pBuffer	= (void *)pBuffer;
	xm_uSize	= uSize;

	xm_bCancelled = false;

	if(xm_bAsyncMode){
		Create(false);
	} else {
		ParasitizeCurrentThread(false, false);
	}
}
void FileClass::xFileThreadClass::Cancel(){
	xm_bCancelled = true;
}
