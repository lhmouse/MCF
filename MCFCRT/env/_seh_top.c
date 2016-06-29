// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "_seh_top.h"

__MCFCRT_C_CDECL
EXCEPTION_DISPOSITION __MCFCRT_SehTopDispatcher(EXCEPTION_RECORD *pRecord, void *pEstablisherFrame, CONTEXT *pContext, void *pDispatcherContext){
	(void)pEstablisherFrame;
	(void)pContext;
	(void)pDispatcherContext;

	const DWORD dwCode = pRecord->ExceptionCode;
	const DWORD dwFlags = pRecord->ExceptionFlags;

	if((dwCode & 0x20FFFFFFul) == 0x20474343ul){ // ('GCC' | (1 << 29))
		if((dwFlags & EXCEPTION_NONCONTINUABLE) == 0){
			return ExceptionContinueExecution;
		}
	}
	return ExceptionContinueSearch;
}
