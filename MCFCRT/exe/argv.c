// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "../MCFCRT.h"
#include "argv.h"
#include <stdlib.h>
#include <wchar.h>
#include <windows.h>

static CRITICAL_SECTION	g_csMutex;
static wchar_t *		g_pwszArgBuffer;
static volatile size_t	g_uArgC;
static ARG_ITEM *		g_pArgV;

unsigned long __MCF_CRT_ExeInitializeArgV(){
	InitializeCriticalSection(&g_csMutex);
	g_pwszArgBuffer = NULL;
	g_uArgC = 0;
	g_pArgV = NULL;

	return ERROR_SUCCESS;
}
void __MCF_CRT_ExeUninitializeArgV(){
	free(g_pArgV);
	g_pArgV = NULL;
	g_uArgC = 0;
	free(g_pwszArgBuffer);
	g_pwszArgBuffer = NULL;
	DeleteCriticalSection(&g_csMutex);
}

size_t MCF_GetArgV(const ARG_ITEM **ppArgV){
	if(!g_pArgV){
		EnterCriticalSection(&g_csMutex);
			if(!g_pArgV){
				const wchar_t *const pwszCommandLine = GetCommandLineW();
				const size_t uCommandLineLen = wcslen(pwszCommandLine);

				g_pwszArgBuffer = malloc((uCommandLineLen + 1) * sizeof(wchar_t));
				if(!g_pwszArgBuffer){
					MCF_CRT_Bail(L"MCF_GetArgV() 失败：内存不足。");
				}
				wmemcpy(g_pwszArgBuffer, pwszCommandLine, uCommandLineLen + 1);

				size_t uIndex = 0;
				size_t uCapacity = 4;
				g_pArgV = malloc(uCapacity * sizeof(ARG_ITEM));
				if(!g_pArgV){
					MCF_CRT_Bail(L"MCF_GetArgV() 失败：内存不足。");
				}

				enum tagState {
					DELIM,
					IN_ARG,
					QUOTE_BEGIN,
					IN_QUOTE,
					QUOTE_END,
					QUOTE_AFTER_QUOTE,
					FINISH
				} eState = DELIM;

				const wchar_t *pwcRead = g_pwszArgBuffer;
				wchar_t *pwcBegin = g_pwszArgBuffer;
				wchar_t *pwcWrite = g_pwszArgBuffer;

				for(;;){
					const wchar_t ch = *(pwcRead++);
					if(ch == 0){
						switch(eState){
						case DELIM:
							// eState = DELIM;
							break;

						case IN_ARG:
						case QUOTE_BEGIN:
						case IN_QUOTE:
						case QUOTE_END:
						case QUOTE_AFTER_QUOTE:
							eState = FINISH;
							break;

						case FINISH:
							eState = DELIM;
							break;
						}
					} else {
						switch(eState){
						case DELIM:
							switch(ch){
							case L' ':
							case L'\t':
								// eState = DELIM;
								break;

							case L'\"':
								eState = QUOTE_BEGIN;
								break;

							default:
								eState = IN_ARG;
								break;
							}
							break;

						case IN_ARG:
							switch(ch){
							case L' ':
							case L'\t':
								eState = FINISH;
								break;

							case L'\"':
								eState = QUOTE_BEGIN;
								break;

							default:
								// eState = IN_ARG;
								break;
							}
							break;

						case QUOTE_BEGIN:
							switch(ch){
							case L'\"':
								eState = QUOTE_END;
								break;

							default:
								eState = IN_QUOTE;
								break;
							}
							break;

						case IN_QUOTE:
							switch(ch){
							case L'\"':
								eState = QUOTE_END;
								break;

							default:
								// eState = IN_QUOTE;
								break;
							}
							break;

						case QUOTE_END:
							switch(ch){
							case L' ':
							case L'\t':
								eState = FINISH;
								break;

							case L'\"':
								eState = QUOTE_AFTER_QUOTE;
								break;

							default:
								eState = IN_ARG;
								break;
							}
							break;

						case QUOTE_AFTER_QUOTE:
							switch(ch){
							case L'\"':
								eState = QUOTE_END;
								break;

							default:
								eState = IN_QUOTE;
								break;
							}
							break;

						case FINISH:
							switch(ch){
							case L' ':
							case L'\t':
								eState = DELIM;
								break;

							case L'\"':
								eState = QUOTE_BEGIN;
								break;

							default:
								eState = IN_ARG;
								break;
							}
							break;
						}
					}
					switch(eState){
					case DELIM:
					case QUOTE_BEGIN:
					case QUOTE_END:
						break;

					case IN_ARG:
					case IN_QUOTE:
					case QUOTE_AFTER_QUOTE:
						*(pwcWrite++) = ch;
						break;

					case FINISH:
						*pwcWrite = 0;
						// 为最后的 NULL 保留一个空位。
						if(uIndex + 2 >= uCapacity){
							uCapacity *= 2;
							ARG_ITEM *const pNewArgV = realloc(g_pArgV, uCapacity * sizeof(ARG_ITEM));
							if(!pNewArgV){
								MCF_CRT_Bail(L"MCF_GetArgV() 失败：内存不足。");
							}
							g_pArgV = pNewArgV;
						}
						g_pArgV[uIndex].pwszBegin = pwcBegin;
						g_pArgV[uIndex].uLen = (size_t)(pwcWrite - pwcBegin);
						++uIndex;

						++pwcWrite;
						pwcBegin = pwcWrite;
						break;
					}
					if(ch == 0){
						g_pArgV[uIndex].pwszBegin = NULL;
						g_pArgV[uIndex].uLen = 0;
						break;
					}
				}

				g_uArgC = uIndex;
			}
		LeaveCriticalSection(&g_csMutex);
	}
	*ppArgV = g_pArgV;
	return g_uArgC;
}
