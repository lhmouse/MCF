// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "HTTPClass.hpp"
#include "xHTTPThreadClass.hpp"
#include <winhttp.h>
#include <vector>
using namespace MCF;

// 静态成员变量定义。
const wchar_t	HTTPClass::xDAY_OF_WEEK_TABLE[][4]	= { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
const wchar_t	HTTPClass::xMONTH_TABLE[][4]		= { L"", L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };

// 静态成员函数。
WCString HTTPClass::xCookieToWCString(LPCWSTR pwszCookieName, const HTTPClass::COOKIEDATA &CookieData){
	WCString wcsRet(pwszCookieName);

	wcsRet += L'=';
	wcsRet += CookieData.wcsValue;
	if(!CookieData.wcsDomain.IsEmpty()){
		wcsRet += L"; Domain=";
		wcsRet += CookieData.wcsDomain;
	}
	if(!CookieData.wcsPath.IsEmpty()){
		wcsRet += L"; Path=";
		wcsRet += CookieData.wcsDomain;
	}
	if(CookieData.stExpires.wYear != 0){
		wcsRet += L"; Expires=";
		wcsRet += xSystemTimeToWCString(CookieData.stExpires);
	}
	if(CookieData.bSecure){
		wcsRet += L"; Secure";
	}
	if(CookieData.bHttpOnly){
		wcsRet += L"; HttpOnly";
	}

	return wcsRet;
}
WCString HTTPClass::xCookieFromWCString(HTTPClass::COOKIEDATA &CookieDataRet, LPCWSTR pwszCookieString){
	if((pwszCookieString == nullptr) || (pwszCookieString[0] == 0)){
		return WCString();
	}

	WCString wcsCookieString(pwszCookieString);
	std::vector<std::pair<WCString, WCString>> vecComponents;

	wchar_t *pContext;
	wchar_t *pszCurrentPart = std::wcstok_r(wcsCookieString, L";", &pContext);
	while(pszCurrentPart != nullptr){
		WCString wcsPart(pszCurrentPart);
		wcsPart.TrimLeft();

		if(!wcsPart.IsEmpty()){
			wchar_t *const pEquPos = std::wcschr(wcsPart, L'=');
			if(pEquPos == nullptr){
				vecComponents.emplace_back(std::make_pair(std::move(wcsPart), WCString()));
			} else {
				*pEquPos = 0;
				WCString wcsValue(pEquPos + 1);
				vecComponents.emplace_back(std::make_pair(std::move(wcsPart), std::move(wcsValue)));
			}
		}

		pszCurrentPart = std::wcstok_r(nullptr, L";", &pContext);
	}

	if(vecComponents.empty() || vecComponents.front().first.IsEmpty()){
		return WCString();
	}

	CookieDataRet.wcsValue = vecComponents.front().second;
	CookieDataRet.wcsDomain.Clear();
	CookieDataRet.wcsPath.Clear();
	CookieDataRet.stExpires.wYear = 0;
	CookieDataRet.bSecure = false;
	CookieDataRet.bHttpOnly = false;

	for(auto iter = vecComponents.cbegin() + 1; iter != vecComponents.cend(); ++iter){
		if(iter->first == L"Domain"){
			CookieDataRet.wcsDomain = iter->second;
		} else if(iter->first == L"Path"){
			CookieDataRet.wcsPath = iter->second;
		} else if(iter->first == L"Expires"){
			CookieDataRet.stExpires = xSystemTimeFromWCString(iter->second);
		} else if(iter->first == L"Secure"){
			CookieDataRet.bSecure = true;
		} else if(iter->first == L"HttpOnly"){
			CookieDataRet.bHttpOnly = true;
		}
	}
	return std::move(vecComponents.front().first);
}
bool HTTPClass::xIsCookieExpired(const HTTPClass::COOKIEDATA &CookieData){
	if(CookieData.stExpires.wYear == 0){
		return false;
	}

	union {
		FILETIME ft;
		ULARGE_INTEGER uli;
	} Now, CookieExpires;

	::GetSystemTimeAsFileTime(&Now.ft);
	::SystemTimeToFileTime(&CookieData.stExpires, &CookieExpires.ft);

	return Now.uli.QuadPart >= CookieExpires.uli.QuadPart;
}

WCString HTTPClass::xSystemTimeToWCString(const SYSTEMTIME &SystemTimeSrc){
	return WCString().Format(
		L"%3s, %02hu-%3s-%04hu %02hu:%02hu:%02hu GMT",
		xDAY_OF_WEEK_TABLE[SystemTimeSrc.wDayOfWeek],
		SystemTimeSrc.wDay,
		xMONTH_TABLE[SystemTimeSrc.wMonth],
		SystemTimeSrc.wYear,
		SystemTimeSrc.wHour,
		SystemTimeSrc.wMinute,
		SystemTimeSrc.wSecond
	);
}
SYSTEMTIME HTTPClass::xSystemTimeFromWCString(LPCWSTR pwszTimeString){
	SYSTEMTIME stRet;

	wchar_t pszDayOfWeek[4], pszMonth[4], pszSuffix[8];
	if((std::swscanf_s(pwszTimeString,
		L"%3s, %02hu-%3s-%04hu %02hu:%02hu:%02hu %7s",
		pszDayOfWeek,
		COUNTOF(pszDayOfWeek),
		&stRet.wDay,
		pszMonth,
		COUNTOF(pszMonth),
		&stRet.wYear,
		&stRet.wHour,
		&stRet.wMinute,
		&stRet.wSecond,
		pszSuffix,
		COUNTOF(pszSuffix)
	) == 8) && (wcscmp(pszSuffix, L"GMT") == 0)){
		for(std::size_t i = 0; i <= 6; ++i){
			if(wcscmp(pszDayOfWeek, xDAY_OF_WEEK_TABLE[i]) == 0){
				stRet.wDayOfWeek = (WORD)i;
				break;
			}
		}
		for(std::size_t i = 1; i <= 12; ++i){
			if(wcscmp(pszMonth, xMONTH_TABLE[i]) == 0){
				stRet.wMonth = (WORD)i;
				break;
			}
		}
		stRet.wMilliseconds = 0;
	} else {
		stRet.wYear = 0;
	}
	return std::move(stRet);
}

WCString HTTPClass::xMIMETypeToWCString(const HTTPClass::MIMETYPE &MIMETypeSrc){
	WCString wcsRet;

	wcsRet += WCString().ConvFrom(MIMETypeSrc.strType);
	wcsRet += L'/';
	wcsRet += WCString().ConvFrom(MIMETypeSrc.strSubType);

	for(auto iter = MIMETypeSrc.mapParameters.cbegin(); iter != MIMETypeSrc.mapParameters.cend(); ++iter){
		wcsRet += L';';
		wcsRet += L' ';
		wcsRet += WCString().ConvFrom(iter->first);
		wcsRet += L'=';
		wcsRet += WCString().ConvFrom(iter->second);
	}
	return std::move(wcsRet);
}
HTTPClass::MIMETYPE HTTPClass::xMIMETypeFromWCString(LPCWSTR pszMIMETypeString){
	MIMETYPE MIMETypeRet;

	if((pszMIMETypeString == nullptr) || (pszMIMETypeString[0] == 0)){
		return std::move(MIMETypeRet);
	}

	WCString wcsMIMETypeString(pszMIMETypeString);
	std::vector<std::pair<WCString, WCString>> vecComponents;

	wchar_t *pContext;
	wchar_t *pszCurrentPart = std::wcstok_r(wcsMIMETypeString, L";", &pContext);
	while(pszCurrentPart != nullptr){
		WCString wcsPart(pszCurrentPart);
		wcsPart.TrimLeft();

		if(!wcsPart.IsEmpty()){
			wchar_t *const pEquPos = std::wcschr(wcsPart, L'=');
			if(pEquPos == nullptr){
				vecComponents.emplace_back(std::make_pair(std::move(wcsPart), WCString()));
			} else {
				*pEquPos = 0;
				WCString wcsValue(pEquPos + 1);
				vecComponents.emplace_back(std::make_pair(std::move(wcsPart), std::move(wcsValue)));
			}
		}

		pszCurrentPart = std::wcstok_r(nullptr, L";", &pContext);
	}

	if(vecComponents.empty() || vecComponents.front().first.IsEmpty()){
		return std::move(MIMETypeRet);
	}

	const std::ptrdiff_t nSlashPos = vecComponents.front().first.IndexOf(L'/');
	if(nSlashPos == WCString::NOT_FOUND){
		MIMETypeRet.strType = std::move(vecComponents.front().first);
		MIMETypeRet.strSubType.Clear();
	} else {
		MIMETypeRet.strType = vecComponents.front().first.Slice(0, nSlashPos);
		MIMETypeRet.strSubType = vecComponents.front().first.Slice(nSlashPos + 1);
	}

	for(auto iter = vecComponents.cbegin() + 1; iter != vecComponents.cend(); ++iter){
		MIMETypeRet.mapParameters[iter->first] = iter->second;
	}

	return std::move(MIMETypeRet);
}

UTF8String HTTPClass::PercentEncode(const char *pszToEncode){
	// 所有的数字、字母（大写的和小写的），以及符号 -（0x2D）、.（0x2E）_（0x5F） 是不需要编码的。
	// 在 Cookie 中波浪线（~）仍然需要编码。为了简单起见这里就统一处理掉了。
	// 注意空格（0x20）被编码成加号。
	// 此处的数据都是 UTF-8 编码的字符串字面量。
	static const char ENCODE_TABLE[0x100][4] = {
		"%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07", "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
		"%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17", "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
		  "+", "%21", "%22", "%23", "%24", "%25", "%26", "%27", "%28", "%29", "%2A", "%2B", "%2C",   "-",   ".", "%2F",
		  "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",   "8",   "9", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
		"%40",   "A",   "B",   "C",   "D",   "E",   "F",   "G",   "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
		  "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",   "X",   "Y",   "Z", "%5B", "%5C", "%5D", "%5E",   "_",
		"%60",   "a",   "b",   "c",   "d",   "e",   "f",   "g",   "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
		  "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",   "x",   "y",   "z", "%7B", "%7C", "%7D", "%7E", "%7F",
		"%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87", "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
		"%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97", "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
		"%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7", "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
		"%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7", "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
		"%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7", "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
		"%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7", "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
		"%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7", "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
		"%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7", "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
	};

	std::string strEncoded;
	const std::size_t uSrcLen = std::strlen(pszToEncode);

	const char *pchRead = pszToEncode;
	const char *const pchEnd = pszToEncode + uSrcLen;
	while(pchRead < pchEnd){
		strEncoded += ENCODE_TABLE[(std::size_t)(unsigned char)*(pchRead++)];
	}

	return UTF8String(strEncoded.c_str());
}
UTF8String HTTPClass::PercentDecode(const char *pszToDecode){
	// 所有的非十六进制字符在表中都对应 -1。
	static const signed char DECODE_TABLE[0x100] = {
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1
	};

	std::string strDecoded;
	const std::size_t uSrcLen = std::strlen(pszToDecode);
	strDecoded.reserve(uSrcLen);

	const char *pchRead = pszToDecode;
	const char *const pchEnd = pszToDecode + uSrcLen;
	while(pchRead < pchEnd){
		const char ch = *(pchRead++);

		if(ch == '+'){
			strDecoded.push_back(' ');
		} else if((ch == '%') && (pchEnd - pchRead >= 2)){
			const signed char ch1stHex = DECODE_TABLE[(std::size_t)(unsigned char)pchRead[0]];
			const signed char ch2ndHex = DECODE_TABLE[(std::size_t)(unsigned char)pchRead[1]];
			if((ch1stHex != -1) && (ch2ndHex != -1)){
				strDecoded.push_back((char)(((ch1stHex << 4) | ch2ndHex) & 0xFF));
			} else {
				strDecoded.push_back(ch);
				strDecoded.push_back(pchRead[0]);
				strDecoded.push_back(pchRead[1]);
			}
			pchRead += 2;
		} else {
			strDecoded.push_back(ch);
		}
	}

	return UTF8String(strDecoded.c_str());
}

// 构造函数和析构函数。
HTTPClass::HTTPClass(bool bAsyncMode) :
	xm_evnHeaderAvailable(true)
{
	xm_pHTTPThread				= new xHTTPThreadClass(this, bAsyncMode);

	xm_abyProxyIP[0]			= 0;

	xm_nResolveTimeout			= 0;
	xm_nConnectTimeout			= 60000;
	xm_nSendTimeout				= 30000;
	xm_nReceiveTimeout			= 30000;

	xm_dwErrorCode				= 0;

	xm_dwBytesContentLength		= INVALID_DWORD_VALUE;
	xm_dwStatusCode				= INVALID_DWORD_VALUE;
}
HTTPClass::~HTTPClass(){
	Shutdown();
	Wait();

	delete xm_pHTTPThread;
}

// 其他非静态成员函数。
void HTTPClass::xInternalFlushCookie(){
	auto iter = xm_mapCookie.begin();
	while(iter != xm_mapCookie.end()){
		if(xIsCookieExpired(iter->second)){
			iter = xm_mapCookie.erase(iter);
		} else {
			++iter;
		}
	}
}

void HTTPClass::xPostCreateRequest(){
}
void HTTPClass::xPostSendRequest(){
}
void HTTPClass::xPreTransferContent(){
}
void HTTPClass::xPostTransferContent(){
}
void HTTPClass::xPreShutdownRequest(){
}

DWORD HTTPClass::PeekStatusCode() const {
	// 返回 HTTPClass::INVALID_DWORD_VALUE 说明出错或未就绪。
	return xm_dwStatusCode;
}

bool HTTPClass::IsHeaderAvailable() const {
	return xm_evnHeaderAvailable.IsClear();
}
void HTTPClass::WaitForHeader() const {
	xm_evnHeaderAvailable.Wait();
}
void HTTPClass::WaitForHeaderTimeout(DWORD dwMilliSeconds) const {
	xm_evnHeaderAvailable.WaitTimeout(dwMilliSeconds);
}
bool HTTPClass::IsIdle() const {
	return xm_pHTTPThread->WaitTimeout(0);
}
void HTTPClass::Wait() const {
	xm_pHTTPThread->Wait();
}
bool HTTPClass::WaitTimeout(DWORD dwMilliSeconds) const {
	return xm_pHTTPThread->WaitTimeout(dwMilliSeconds);
}

const HTTPClass::MIMETYPE &HTTPClass::GetContentType() const {
	WaitForHeader();
	return xm_ContentType;
}
DWORD HTTPClass::GetBytesTotal() const {
	// 返回 HTTPClass::INVALID_DWORD_VALUE 说明出错，返回 0 说明 HTTP 头有效，但是其中没有给出长度。
	WaitForHeader();
	return xm_dwBytesContentLength;
}
DWORD HTTPClass::GetBytesReceived() const {
	WaitForHeader();
	xm_rwlReceivedDataLock.AcquireReadLock();
		const DWORD dwBytesReceived = (DWORD)xm_vecbyReceivedData.size();
	xm_rwlReceivedDataLock.ReleaseReadLock();
	return dwBytesReceived;
}
DWORD HTTPClass::GetStatusCode() const {
	WaitForHeader();
	return PeekStatusCode();
}

void HTTPClass::SetUserAgent(LPCTSTR pszNewUserAgent){
	Wait();
	xm_wcsUserAgent.ConvFrom(TString(pszNewUserAgent));
}
void HTTPClass::SetProxy(BYTE byIP1, BYTE byIP2, BYTE byIP3, BYTE byIP4, DWORD dwPort, LPCTSTR pszUserName, LPCTSTR pszPassword){
	Wait();

	xm_abyProxyIP[0]	= byIP1;
	xm_abyProxyIP[1]	= byIP2;
	xm_abyProxyIP[2]	= byIP3;
	xm_abyProxyIP[3]	= byIP4;
	xm_dwProxyPort		= dwPort;
	xm_wcsProxyUserName	= pszUserName;
	xm_wcsProxyPassword	= pszPassword;
}
void HTTPClass::ClearProxy(){
	Wait();

	xm_abyProxyIP[0]	= 0;
}
void HTTPClass::SetTimeouts(int nResolveTimeout, int nConnectTimeout, int nSendTimeout, int nReceiveTimeout){
	Wait();
	xm_nResolveTimeout	= nResolveTimeout;
	xm_nConnectTimeout	= nConnectTimeout;
	xm_nSendTimeout		= nSendTimeout;
	xm_nReceiveTimeout	= nReceiveTimeout;
}

DWORD HTTPClass::GetErrorCode() const {
	Wait();
	return xm_dwErrorCode;
}

TString HTTPClass::GetHeader(LPCTSTR pszHeaderName) const {
	Wait();

	const auto iter = xm_mapHeaders.find(pszHeaderName);
	if(iter == xm_mapHeaders.end()){
		return TString();
	} else {
		return iter->second;
	}
}

const BYTE *HTTPClass::GetDataPtr() const {
	Wait();
	return xm_vecbyReceivedData.data();
}
DWORD HTTPClass::GetDataSize() const {
	Wait();
	return xm_vecbyReceivedData.size();
}

void HTTPClass::FlushCookie(){
	Wait();
	xInternalFlushCookie();
}
void HTTPClass::ClearCookie(){
	Wait();
	xm_mapCookie.clear();
}

TString HTTPClass::ExportCookie(bool bIgnoreSessionOnly, bool bIgnoreHttpOnly) const {
	Wait();

	WCString wcsRet;
	for(auto iter = xm_mapCookie.cbegin(); iter != xm_mapCookie.cend(); ++iter){
		if(!(bIgnoreSessionOnly && (iter->second.stExpires.wYear == 0)) && !(bIgnoreHttpOnly && iter->second.bHttpOnly)){
			wcsRet += std::move(xCookieToWCString(iter->first, iter->second));
			wcsRet += L"\r\n";
		}
	}
	return TString().ConvFrom(wcsRet);
}
void HTTPClass::ImportCookie(LPCTSTR pszExportedCookie){
	Wait();

	WCString wcsExportedCookie(WCString().ConvFrom(TString(pszExportedCookie)));
	wcsExportedCookie.ReplaceAll(L"\r\n", L"\n");

	wchar_t *pContext;
	wchar_t *pszCurrentLine = std::wcstok_r(wcsExportedCookie, L"\n", &pContext);
	while(pszCurrentLine != nullptr){
		WCString wcsLine(pszCurrentLine);
		wcsLine.TrimLeft();

		if(!wcsLine.IsEmpty()){
			COOKIEDATA CookieData;
			WCString wcsCookieName(xCookieFromWCString(CookieData, wcsLine));
			if(!wcsCookieName.IsEmpty()){
				xm_mapCookie[wcsCookieName] = std::move(CookieData);
			}
		}

		pszCurrentLine = std::wcstok_r(nullptr, L";", &pContext);
	}
}

TString HTTPClass::GetCookie(LPCTSTR pszName) const {
	Wait();

	const auto iter = xm_mapCookie.find(WCString().ConvFrom(TString(pszName)));
	if(iter == xm_mapCookie.end()){
		return TString();
	}
	return TString().ConvFrom(PercentDecode(UTF8String().ConvFrom(iter->second.wcsValue)));
}
void HTTPClass::SetCookie(LPCTSTR pszName, LPCTSTR pszValue){
	Wait();

	if((pszName == nullptr) || (pszName[0] == 0)){
		return;
	}

	COOKIEDATA CookieData;
	CookieData.wcsValue			= WCString().ConvFrom(TString(pszValue));
	CookieData.wcsDomain		= L"";
	CookieData.wcsPath			= L"";
	CookieData.stExpires.wYear	= 0;
	CookieData.bSecure			= false;
	CookieData.bHttpOnly		= false;

	xm_mapCookie[WCString().ConvFrom(TString(pszName))] = CookieData;
}

const HTTPClass::COOKIEMAP &HTTPClass::ExportRawCookie() const {
	Wait();
	return xm_mapCookie;
}
void HTTPClass::ImportRawCookie(HTTPClass::COOKIEMAP &&mapExportedCookie){
	Wait();

	for(auto iter = mapExportedCookie.begin(); iter != mapExportedCookie.end(); ++iter){
		xm_mapCookie[iter->first] = std::move(iter->second);
	}
	mapExportedCookie.clear();
}
bool HTTPClass::GetRawCookie(HTTPClass::COOKIEDATA &CookieDataRet, LPCWSTR pwszName) const {
	Wait();

	const auto iter = xm_mapCookie.find(pwszName);
	if(iter == xm_mapCookie.end()){
		return false;
	}
	CookieDataRet = iter->second;
	return true;
}
void HTTPClass::SetRawCookie(LPCWSTR pwszName, HTTPClass::COOKIEDATA &&CookieData){
	Wait();
	xm_mapCookie[pwszName] = CookieData;
}

bool HTTPClass::Get(LPCTSTR pszURL){
	Wait();
	return xm_pHTTPThread->CreateRequest(pszURL, L"GET", nullptr, 0, nullptr);
}
bool HTTPClass::Head(LPCTSTR pszURL){
	Wait();
	return xm_pHTTPThread->CreateRequest(pszURL, L"HEAD", nullptr, 0, nullptr);
}
bool HTTPClass::Post(LPCTSTR pszURL, const void *pDataToPost, std::size_t uBytesToPost, LPCTSTR pszContentType){
	Wait();
	return xm_pHTTPThread->CreateRequest(pszURL, L"POST", pDataToPost, uBytesToPost, pszContentType);
}
void HTTPClass::Shutdown(){
	xm_pHTTPThread->Cancel();
}
