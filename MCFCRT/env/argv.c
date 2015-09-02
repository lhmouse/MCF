// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "argv.h"
#include "mcfwin.h"
#include <stdlib.h>
#include <wchar.h>

/*
	这里的内存布局可以如此表示：

	struct Storage {
		wchar_t cmdline[];		// 动态确定。
		MCF_ArgItem stub;		// pwszStr 指向 cmdline，uLen 是后面的 argv 的元素容量。
		MCF_ArgItem argv[];		// MCF_CRT_AllocArgv 返回的指针指向这里。
		MCF_ArgItem nil;		// pwszStr 为 nullptr，uLen 为 0。
	};
*/

const MCF_ArgItem *MCF_CRT_AllocArgv(size_t *pArgc, const wchar_t *pwszCommandLine){
	const size_t uPrefixSize = (((wcslen(pwszCommandLine) + 1) * sizeof(wchar_t) - 1) / alignof(MCF_ArgItem) + 1) * alignof(MCF_ArgItem);

	size_t uCapacity = 4;
	const size_t uSizeToAlloc = uPrefixSize + (uCapacity + 2) * sizeof(MCF_ArgItem);
	if((uSizeToAlloc < uPrefixSize) || (uSizeToAlloc >= (SIZE_MAX >> 2))){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
	void *pStorage = malloc(uSizeToAlloc);
	if(!pStorage){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
	MCF_ArgItem *pArgv = (MCF_ArgItem *)((char *)pStorage + uPrefixSize + sizeof(MCF_ArgItem));
	pArgv[-1].pwszStr = pStorage;
	pArgv[-1].uLen = uCapacity;

	const wchar_t *pwcRead = pwszCommandLine;
	wchar_t *pwcWrite = pStorage;
	size_t uArgc = 0;
	*pArgc = 0;

	enum {
		ST_DELIM,
		ST_IN_ARG,
		ST_QUOTE_OPEN,
		ST_IN_QUOTE,
		ST_QUOTE_CLOSED,
	} eState = ST_DELIM;

	for(;;){
		const wchar_t wc = *(pwcRead++);
		if(wc == 0){
			break;
		}
		switch(eState){
		case ST_DELIM:
			if((wc == L' ') || (wc == L'\t')){
				// eState = ST_DELIM;
			} else {
				if(uArgc == uCapacity){
					uCapacity = uCapacity * 3 / 2;
					const size_t uNewSizeToAlloc = uPrefixSize + (uCapacity + 2) * sizeof(MCF_ArgItem);
					if((uNewSizeToAlloc <= uSizeToAlloc) || (uNewSizeToAlloc >= (SIZE_MAX >> 2))){
						goto jBadAlloc;
					}
					void *pNewStorage = realloc(pStorage, uNewSizeToAlloc);
					if(!pNewStorage){
						goto jBadAlloc;
					}
					MCF_ArgItem *pArgv = (MCF_ArgItem *)((char *)pNewStorage + uPrefixSize + sizeof(MCF_ArgItem));
					pArgv[-1].pwszStr = pNewStorage;
					pArgv[-1].uLen = uCapacity;

					pwcWrite = (wchar_t *)pNewStorage + (pwcWrite - (wchar_t *)pStorage);

					pStorage = pNewStorage;
				}
				++uArgc;
				pArgv[uArgc - 1].pwszStr = pwcWrite;

				if(wc == L'\"'){
					eState = ST_QUOTE_OPEN;
				} else {
					*(pwcWrite++) = wc;
					eState = ST_IN_ARG;
				}
			}
			break;

		case ST_IN_ARG:
			if((wc == L' ') || (wc == L'\t')){
				pArgv[uArgc - 1].uLen = (size_t)(pwcWrite - pArgv[uArgc - 1].pwszStr);
				*(pwcWrite++) = 0;
				eState = ST_DELIM;
			} else if(wc == L'\"'){
				eState = ST_QUOTE_OPEN;
			} else {
				*(pwcWrite++) = wc;
				// eState = ST_IN_ARG;
			}
			break;

		case ST_QUOTE_OPEN:
			if(wc == L'\"'){
				eState = ST_QUOTE_CLOSED;
			} else {
				*(pwcWrite++) = wc;
				eState = ST_IN_QUOTE;
			}
			break;

		case ST_IN_QUOTE:
			if(wc == L'\"'){
				eState = ST_QUOTE_CLOSED;
			} else {
				*(pwcWrite++) = wc;
				// eState = ST_IN_QUOTE;
			}
			break;

		case ST_QUOTE_CLOSED:
			if((wc == L' ') || (wc == L'\t')){
				pArgv[uArgc - 1].uLen = (size_t)(pwcWrite - pArgv[uArgc - 1].pwszStr);
				*(pwcWrite++) = 0;
				eState = ST_DELIM;
			} else if(wc == L'\"'){
				*(pwcWrite++) = wc;
				eState = ST_IN_QUOTE;
			} else {
				*(pwcWrite++) = wc;
				eState = ST_IN_ARG;
			}
			break;
		}
	}
	switch(eState){
	case ST_DELIM:
		break;

	case ST_IN_ARG:
	case ST_QUOTE_OPEN:
	case ST_IN_QUOTE:
	case ST_QUOTE_CLOSED:
		pArgv[uArgc - 1].uLen = (size_t)(pwcWrite - pArgv[uArgc - 1].pwszStr);
		*(pwcWrite++) = 0;
		break;
	}

	pArgv[uArgc].pwszStr = nullptr;
	pArgv[uArgc].uLen = 0;

	*pArgc = uArgc;
	return pArgv;

jBadAlloc:
	free(pStorage);
	SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	return nullptr;
}

const MCF_ArgItem *MCF_CRT_AllocArgvFromCommandLine(size_t *pArgc){
	return MCF_CRT_AllocArgv(pArgc, GetCommandLineW());
}
void MCF_CRT_FreeArgv(const MCF_ArgItem *pArgItems){
	if(pArgItems){
		free((void *)(pArgItems[-1].pwszStr));
	}
}
