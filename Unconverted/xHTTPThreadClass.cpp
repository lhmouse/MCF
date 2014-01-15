// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "Memory.hpp"
#include "xHTTPThreadClass.hpp"
#include <winhttp.h>
#include <vector>
using namespace MCF;

// 静态成员函数。
DWORD HTTPClass::xHTTPThreadClass::xSelectAuthScheme(DWORD dwSupported, DWORD dwFirst){
	const DWORD dwAllSchemes = WINHTTP_AUTH_SCHEME_BASIC | WINHTTP_AUTH_SCHEME_NTLM | WINHTTP_AUTH_SCHEME_PASSPORT | WINHTTP_AUTH_SCHEME_DIGEST | WINHTTP_AUTH_SCHEME_NEGOTIATE;

	dwSupported &= dwAllSchemes;
	dwFirst &= dwAllSchemes;

	const DWORD dwChooseFrom = (dwFirst == 0) ? dwSupported : dwFirst;
	if((dwChooseFrom & WINHTTP_AUTH_SCHEME_NEGOTIATE) != 0){
		return WINHTTP_AUTH_SCHEME_NEGOTIATE;
	} else if((dwChooseFrom & WINHTTP_AUTH_SCHEME_DIGEST) != 0){
		return WINHTTP_AUTH_SCHEME_DIGEST;
	} else if((dwChooseFrom & WINHTTP_AUTH_SCHEME_PASSPORT) != 0){
		return WINHTTP_AUTH_SCHEME_PASSPORT;
	} else if((dwChooseFrom & WINHTTP_AUTH_SCHEME_NTLM) != 0){
		return WINHTTP_AUTH_SCHEME_NTLM;
	} else {
		return WINHTTP_AUTH_SCHEME_BASIC;
	}
}

// 构造函数和析构函数。
HTTPClass::xHTTPThreadClass::xHTTPThreadClass(HTTPClass *pHTTP, bool bAsyncMode) :
	xm_pHTTP		(pHTTP),
	xm_bAsyncMode	(bAsyncMode)
{
}
HTTPClass::xHTTPThreadClass::~xHTTPThreadClass(){
}

// 其他非静态成员函数。
int HTTPClass::xHTTPThreadClass::xThreadProc(){
	DWORD dwProxySupportedSchemes = 0;
	DWORD dwProxyFirstScheme = 0;
	DWORD dwProxyAuthTarget = 0;

	DWORD dwSupportedSchemes = 0;
	DWORD dwFirstScheme = 0;
	DWORD dwAuthTarget = 0;

	try {
		MCF::UniqueHandleTemplate<HINTERNET, NULL, decltype(&::WinHttpCloseHandle)> hSession, hConnect, hRequest;

	retry:
		if(xm_pHTTP->xm_abyProxyIP[0] == 0){
			hSession.Assign(::WinHttpOpen(xm_pHTTP->xm_wcsUserAgent, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, nullptr, 0), &::WinHttpCloseHandle);
		} else {
			hSession.Assign(
				::WinHttpOpen(xm_pHTTP->xm_wcsUserAgent, WINHTTP_ACCESS_TYPE_NAMED_PROXY,
					WCString().Format(L"%hhu.%hhu.%hhu.%hhu:%lu", xm_pHTTP->xm_abyProxyIP[0], xm_pHTTP->xm_abyProxyIP[1], xm_pHTTP->xm_abyProxyIP[2], xm_pHTTP->xm_abyProxyIP[3], (unsigned long)xm_pHTTP->xm_dwProxyPort),
					nullptr,
					0
				),
				&::WinHttpCloseHandle
			);

			if((dwProxySupportedSchemes != 0) && !xm_pHTTP->xm_wcsProxyUserName.IsEmpty()){
				if(::WinHttpSetCredentials(hRequest, dwProxyAuthTarget, xSelectAuthScheme(dwProxySupportedSchemes, dwProxyFirstScheme), xm_pHTTP->xm_wcsProxyUserName, xm_pHTTP->xm_wcsProxyPassword, nullptr) == FALSE){
					throw ::GetLastError();
				}
			}
		}
		if(hSession == NULL){
			throw ::GetLastError();
		}

		hConnect.Assign(::WinHttpConnect(hSession, xm_wcsHostName, xm_wPort, 0), &::WinHttpCloseHandle);
		if(hConnect == NULL){
			throw ::GetLastError();
		}

		if(xm_nScheme == INTERNET_SCHEME_HTTPS){
			hRequest.Assign(::WinHttpOpenRequest(hConnect, xm_wcsVerb, xm_wcsPath + xm_wcsExtraInfo, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE), &::WinHttpCloseHandle);
		} else {
			hRequest.Assign(::WinHttpOpenRequest(hConnect, xm_wcsVerb, xm_wcsPath + xm_wcsExtraInfo, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0), &::WinHttpCloseHandle);
		}
		if(hRequest == NULL){
			throw ::GetLastError();
		}

		if(::WinHttpSetTimeouts(hRequest, xm_pHTTP->xm_nResolveTimeout, xm_pHTTP->xm_nConnectTimeout, xm_pHTTP->xm_nSendTimeout, xm_pHTTP->xm_nReceiveTimeout) == FALSE){
			throw ::GetLastError();
		}

		if((dwSupportedSchemes != 0) && !xm_wcsUserName.IsEmpty()){
			if(::WinHttpSetCredentials(hRequest, dwAuthTarget, xSelectAuthScheme(dwSupportedSchemes, dwFirstScheme), xm_wcsUserName, xm_wcsPassword, nullptr) == FALSE){
				throw ::GetLastError();
			}
		}

		xm_pHTTP->xPostCreateRequest();

		WCString wcsAdditionalHeaders;
		xm_pHTTP->xInternalFlushCookie();
		auto iter =xm_pHTTP-> xm_mapCookie.cbegin();
		if(iter != xm_pHTTP->xm_mapCookie.end()){
			wcsAdditionalHeaders = L"Cookie: ";
			wcsAdditionalHeaders += xCookieToWCString(iter->first, iter->second);

			while(++iter != xm_pHTTP->xm_mapCookie.end()){
				wcsAdditionalHeaders += L"; ";
				wcsAdditionalHeaders += xCookieToWCString(iter->first, iter->second);
			}
			wcsAdditionalHeaders += L"\r\n";
		}

		if(!xm_wcsPostMIMEType.IsEmpty()){
			wcsAdditionalHeaders += L"Content-Type: ";
			wcsAdditionalHeaders += xm_wcsPostMIMEType;
			wcsAdditionalHeaders += L"\r\n";
		}

		if(::WinHttpSendRequest(hRequest, wcsAdditionalHeaders.GetBuffer(), (DWORD)wcsAdditionalHeaders.GetLength(), xm_vecbyPostData.data(), (DWORD)xm_vecbyPostData.size(), (DWORD)xm_vecbyPostData.size(), 0) == FALSE){
			throw ::GetLastError();
		}

		xm_pHTTP->xPostSendRequest();

		if(::WinHttpReceiveResponse(hRequest, nullptr) == FALSE){
			throw ::GetLastError();
		}

		WCString wcsHeaderBuffer;
		auto QueryHeaders = [&](DWORD dwInfoLevel) -> WCString & {
			DWORD dwHeaderLength;
			::WinHttpQueryHeaders(hRequest, dwInfoLevel, WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &dwHeaderLength, nullptr);
			if(::GetLastError() == ERROR_INSUFFICIENT_BUFFER){
				wcsHeaderBuffer.Resize(dwHeaderLength);
				::WinHttpQueryHeaders(hRequest, dwInfoLevel, WINHTTP_HEADER_NAME_BY_INDEX, wcsHeaderBuffer.GetBuffer(), &dwHeaderLength, nullptr);
			} else {
				wcsHeaderBuffer.Clear();
			}
			return wcsHeaderBuffer;
		};
		auto QueryHeaderAsDWord = [&](DWORD dwInfoLevel) -> DWORD {
			DWORD dwRet;
			DWORD dwHeaderLength = sizeof(DWORD);
			if(::WinHttpQueryHeaders(hRequest, dwInfoLevel | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &dwRet, &dwHeaderLength, nullptr) != FALSE){
				return dwRet;
			} else {
				return 0;
			}
		};
		auto QueryHeadersByIndex = [&](DWORD dwInfoLevel, DWORD *pdwIndex) -> WCString & {
			DWORD dwHeaderLength;
			::WinHttpQueryHeaders(hRequest, dwInfoLevel, WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &dwHeaderLength, pdwIndex);
			if(::GetLastError() == ERROR_INSUFFICIENT_BUFFER){
				wcsHeaderBuffer.Resize(dwHeaderLength);
				::WinHttpQueryHeaders(hRequest, dwInfoLevel, WINHTTP_HEADER_NAME_BY_INDEX, wcsHeaderBuffer.GetBuffer(), &dwHeaderLength, pdwIndex);
			} else {
				wcsHeaderBuffer.Clear();
			}
			return wcsHeaderBuffer;
		};

		LPCWSTR pwszCurrentHeader = QueryHeaders(WINHTTP_QUERY_RAW_HEADERS);
		for(;;){
			const std::size_t uHeaderLen = std::wcslen(pwszCurrentHeader);
			if(pwszCurrentHeader[uHeaderLen + 1] == 0){
				break;
			}

			const LPCWSTR pwchColonPos = std::wcschr(pwszCurrentHeader, L':');
			if(pwchColonPos != nullptr){
				xm_pHTTP->xm_mapHeaders.emplace(std::make_pair(WCString(pwszCurrentHeader, pwchColonPos), WCString(pwchColonPos + 1).TrimLeft()));
			}
			pwszCurrentHeader += uHeaderLen + 1;
		}

		xm_pHTTP->xm_dwStatusCode = QueryHeaderAsDWord(WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER);
		xm_pHTTP->xm_dwBytesContentLength = QueryHeaderAsDWord(WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER);
		xm_pHTTP->xm_vecbyReceivedData.reserve(xm_pHTTP->xm_dwBytesContentLength);
		xm_pHTTP->xm_ContentType = xMIMETypeFromWCString(QueryHeaders(WINHTTP_QUERY_CONTENT_TYPE));

		DWORD dwCurrentSetCookieIndex = 0;
		for(;;){
			const WCString wcsCurrentSetCookie = QueryHeadersByIndex(WINHTTP_QUERY_SET_COOKIE, &dwCurrentSetCookieIndex);
			if(wcsCurrentSetCookie.IsEmpty()){
				break;
			}

			COOKIEDATA CookieData;
			WCString wcsCookieName(xCookieFromWCString(CookieData, wcsCurrentSetCookie));
			if(!wcsCookieName.IsEmpty()){
				xm_pHTTP->xm_mapCookie[wcsCookieName] = CookieData;
			}
			++dwCurrentSetCookieIndex;
		}
		xm_pHTTP->xInternalFlushCookie();

		xm_pHTTP->xm_evnHeaderAvailable.Set();

		xm_pHTTP->xPreTransferContent();
		DWORD dwBytesToRead;
		DWORD dwBytesRead;
		do {
			if(::WinHttpQueryDataAvailable(hRequest, &dwBytesToRead) == FALSE){
				throw ::GetLastError();
			}

			const std::size_t uOldSize = xm_pHTTP->xm_vecbyReceivedData.size();
			xm_pHTTP->xm_vecbyReceivedData.resize(uOldSize + (std::size_t)dwBytesToRead);
			if(::WinHttpReadData(hRequest, xm_pHTTP->xm_vecbyReceivedData.data() + (std::ptrdiff_t)uOldSize, dwBytesToRead, &dwBytesRead) == FALSE){
				throw ::GetLastError();
			}
		} while(dwBytesRead != 0);

		if((xm_pHTTP->xm_dwStatusCode == 407) && (dwProxySupportedSchemes == 0)){
			if(::WinHttpQueryAuthSchemes(hRequest, &dwProxySupportedSchemes, &dwProxyFirstScheme, &dwProxyAuthTarget) == FALSE){
				throw ::GetLastError();
			}
			if(dwProxySupportedSchemes != 0){
				goto retry;
			}
		} else if((xm_pHTTP->xm_dwStatusCode == 401) && (dwSupportedSchemes == 0)){
			if(::WinHttpQueryAuthSchemes(hRequest, &dwSupportedSchemes, &dwFirstScheme, &dwAuthTarget) == FALSE){
				throw ::GetLastError();
			}
			if(dwSupportedSchemes != 0){
				goto retry;
			}
		}

		xm_pHTTP->xm_dwErrorCode = ERROR_SUCCESS;
	} catch(DWORD dwErrorCode){
		xm_pHTTP->xm_evnHeaderAvailable.Set();
		xm_pHTTP->xm_dwErrorCode = dwErrorCode;
	}

	xm_pHTTP->xPreShutdownRequest();

	return 0;
}

bool HTTPClass::xHTTPThreadClass::CreateRequest(LPCTSTR pszURL, LPCWSTR pwszVerb, const void *pDataToPost, std::size_t uBytesToPost, LPCTSTR pszPostMIMEType){
	const std::size_t uURLSize = _tcslen(pszURL) + 1;
	xm_wcsHostName	.Resize(uURLSize);
	xm_wcsUserName	.Resize(uURLSize);
	xm_wcsPassword	.Resize(uURLSize);
	xm_wcsPath		.Resize(uURLSize);
	xm_wcsExtraInfo	.Resize(uURLSize);

	URL_COMPONENTS URLComponents;
	URLComponents.dwStructSize		= sizeof(URLComponents);
	URLComponents.lpszScheme		= nullptr;
	URLComponents.dwSchemeLength	= 0;
	URLComponents.lpszHostName		= xm_wcsHostName;
	URLComponents.dwHostNameLength	= uURLSize;
	URLComponents.lpszUserName		= xm_wcsUserName;
	URLComponents.dwUserNameLength	= uURLSize;
	URLComponents.lpszPassword		= xm_wcsPassword;
	URLComponents.dwPasswordLength	= uURLSize;
	URLComponents.lpszUrlPath		= xm_wcsPath;
	URLComponents.dwUrlPathLength	= uURLSize;
	URLComponents.lpszExtraInfo		= xm_wcsExtraInfo;
	URLComponents.dwExtraInfoLength	= uURLSize;

	if(::WinHttpCrackUrl(WCString().ConvFrom(pszURL), 0, 0, &URLComponents) == FALSE){
		xm_pHTTP->xm_dwErrorCode = ERROR_INVALID_PARAMETER;
		return false;
	}

	xm_nScheme = URLComponents.nScheme;
	xm_wPort = URLComponents.nPort;
	if(xm_wcsPath.IsEmpty()){
		xm_wcsPath = L"/";
	}

	xm_wcsVerb = pwszVerb;
	if(uBytesToPost == 0){
		xm_vecbyPostData.clear();
		xm_wcsPostMIMEType.Clear();
	} else {
		xm_vecbyPostData.assign((const BYTE *)pDataToPost, ((const BYTE *)pDataToPost) + uBytesToPost);
		xm_wcsPostMIMEType = pszPostMIMEType;
	}

	xm_bCancelled = false;

	xm_pHTTP->xm_evnHeaderAvailable.Clear();
	xm_pHTTP->xm_dwErrorCode			= ERROR_SUCCESS;
	xm_pHTTP->xm_dwBytesContentLength	= INVALID_DWORD_VALUE;
	xm_pHTTP->xm_dwStatusCode			= INVALID_DWORD_VALUE;
	xm_pHTTP->xm_mapHeaders.clear();
	xm_pHTTP->xm_vecbyReceivedData.clear();
	xm_pHTTP->xm_ContentType.strType.Clear();

	if(xm_bAsyncMode){
		Create(false);
	} else {
		ParasitizeCurrentThread(false, false);
	}

	return true;
}
void HTTPClass::xHTTPThreadClass::Cancel(){
	xm_bCancelled = true;
}
