// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "HttpClient.hpp"
#include "../../MCFCRT/c/ext/assert.h"
#include "../../MCFCRT/cpp/ext/multi_indexed_map.hpp"
#include "../../MCFCRT/cpp/ext/vvector.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/SharedHandleNts.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/VarIntEx.hpp"
#include <algorithm>
#include <winhttp.h>
using namespace MCF;
/*
// 外部函数定义。
namespace MCF {

// http://tools.ietf.org/html/rfc3986
void UrlEncode(Utf8String &u8sAppendTo, const char *pchBegin, std::size_t uLen){
	static constexpr std::uint32_t TABLE[8] = {
		0xFFFFFFFF,	// 0x00 - 0x1F
		0xFC009FFF,	// 0x20 - 0x3F
		0x78000001,	// 0x40 - 0x5F
		0xB8000001,	// 0x60 - 0x7F
		0xFFFFFFFF,	// 0x80 - 0x9F
		0xFFFFFFFF,	// 0xA0 - 0xBF
		0xFFFFFFFF,	// 0xC0 - 0xDF
		0xFFFFFFFF	// 0xE0 - 0xFF
	};

	if(uLen == (std::size_t)-1){
		uLen = StrLen(pchBegin);
	}
	u8sAppendTo.Reserve(u8sAppendTo.GetSize() + uLen * 5 / 4);

	auto pchRead = pchBegin;
	const auto pchEnd = pchBegin + uLen;
	while(pchRead != pchEnd){
		const unsigned char uch = *(pchRead++);
		if(uch == ' '){
			u8sAppendTo += '+';
		} else if(TABLE[uch >> 5] & (1u << (uch & 0x1F))){
			u8sAppendTo += '%';
			char chEncoded = (uch >> 4) + '0';
			if(chEncoded > '9'){
				chEncoded += 'A' - ('9' + 1);
			}
			u8sAppendTo += chEncoded;
			chEncoded = (uch & 0x0F) + '0';
			if(chEncoded > '9'){
				chEncoded += 'A' - ('9' + 1);
			}
			u8sAppendTo += chEncoded;
		} else {
			u8sAppendTo += uch;
		}
	}
}
void UrlEncode(Utf8String &u8sAppendTo, const Utf8String &u8sSrc){
	UrlEncode(u8sAppendTo, u8sSrc.GetCStr(), u8sSrc.GetLength());
}
void UrlDecode(Utf8String &u8sAppendTo, const char *pchBegin, std::size_t uLen){
	static constexpr unsigned char TABLE[256] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

	if(uLen == (std::size_t)-1){
		uLen = StrLen(pchBegin);
	}
	u8sAppendTo.Reserve(u8sAppendTo.GetSize() + uLen);

	auto pchRead = pchBegin;
	const auto pchEnd = pchBegin + uLen;
	while(pchRead != pchEnd){
		const char ch = *(pchRead++);
		if(ch != '%'){
			if(ch == '+'){
				u8sAppendTo += ' ';
			} else {
				u8sAppendTo += ch;
			}
			continue;
		}
		const unsigned char uch2 = *(pchRead++);
		const unsigned char uchHigh = TABLE[uch2];
		if(uchHigh == 0xFF){
			u8sAppendTo += '%';
			u8sAppendTo += uch2;
			continue;
		}
		const unsigned char uch3 = *(pchRead++);
		const unsigned char uchLow = TABLE[uch3];
		if(uchLow == 0xFF){
			u8sAppendTo += '%';
			u8sAppendTo += uch2;
			u8sAppendTo += uch3;
			continue;
		}
		u8sAppendTo += (uchHigh << 4) | uchLow;
	}
}
void UrlDecode(Utf8String &u8sAppendTo, const Utf8String &u8sSrc){
	UrlDecode(u8sAppendTo, u8sSrc.GetCStr(), u8sSrc.GetLength());
}

}

// 嵌套类定义。
class HttpClient::xDelegate : NO_COPY {
private:
	struct WinHttpCloser {
		constexpr HINTERNET operator()() const noexcept {
			return NULL;
		}
		void operator()(HINTERNET hInternet) const noexcept {
			::WinHttpCloseHandle(hInternet);
		}
	};
	typedef UniqueHandle<WinHttpCloser>		xWinHttpHandle;
	typedef SharedHandleNts<WinHttpCloser>	xSharedWinHttpHandle;

	struct GlobalFreer {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *pGlobal) const noexcept {
			::GlobalFree((HGLOBAL)pGlobal);
		}
	};
	typedef UniqueHandle<GlobalFreer> xGlobalMem;

	// http://tools.ietf.org/html/rfc6265
	struct DomainPathNameComp;

	typedef MultiIndexedMap<
		Utf8String,		// Value
		Utf8String,		// Name
		std::uint64_t,	// Expires
		Utf8String,		// Path
		Utf8String,		// Domain	这里的字符串是反序的，使用 $ 后缀表示完全匹配，. 后缀表示部分匹配。
						//			例如来自“www.foo.bar.com”未指定 domain 的 cookie 存储为“moc.rab.oof.www$”，
						//			指定“foo.bar.com”的 cookie 存储为“moc.rab.oof.”。
		bool,			// Secure
		bool,			// HttpOnly
		Index<int, DomainPathNameComp>
	> xCookieMap;

	static const xCookieMap::Node NULL_COOKIE_NODE;

	enum : std::size_t {
		IDX_NAME,
		IDX_EXPIRES,
		IDX_PATH,
		IDX_DOMAIN,
		IDX_SECURE,
		IDX_HTTP_ONLY,
		IDX_DOMAIN_PATH_NAME
	};

	class DomainPathNameComp {
	private:
		static constexpr const xCookieMap::Node &xGetNode(const int &nIndex) noexcept {
			return DOWN_CAST(const xCookieMap::Node, GetIndex<IDX_DOMAIN_PATH_NAME>(), &nIndex)[0];
		}

	public:
		bool operator()(const int &lhs, const int &rhs) const noexcept {
			const auto &vLNode = xGetNode(lhs);
			const auto &vRNode = xGetNode(rhs);

			int nResult = vLNode.GetIndex<IDX_DOMAIN>().Compare(vRNode.GetIndex<IDX_DOMAIN>());
			if(nResult != 0){
				return nResult < 0;
			}
			nResult = vLNode.GetIndex<IDX_PATH>().Compare(vRNode.GetIndex<IDX_PATH>());
			if(nResult != 0){
				return nResult < 0;
			}
			return vLNode.GetIndex<IDX_NAME>().Compare(vRNode.GetIndex<IDX_NAME>()) < 0;
		}
		bool operator()(const int &lhs, const std::pair<const char *, std::size_t> (&rhs)[3]) const noexcept {
			const auto &vLNode = xGetNode(lhs);

			int nResult = vLNode.GetIndex<IDX_DOMAIN>().Compare(rhs[0].first, rhs[0].second);
			if(nResult != 0){
				return nResult < 0;
			}
			nResult = vLNode.GetIndex<IDX_PATH>().Compare(rhs[1].first, rhs[1].second);
			if(nResult != 0){
				return nResult < 0;
			}
			return vLNode.GetIndex<IDX_NAME>().Compare(rhs[2].first, rhs[2].second) < 0;
		}
		bool operator()(const std::pair<const char *, std::size_t> (&lhs)[3], const int &rhs) const noexcept {
			const auto &vRNode = xGetNode(rhs);

			int nResult = vRNode.GetIndex<IDX_DOMAIN>().Compare(lhs[0].first, lhs[0].second);
			if(nResult != 0){
				return nResult > 0;
			}
			nResult = vRNode.GetIndex<IDX_PATH>().Compare(lhs[1].first, lhs[1].second);
			if(nResult != 0){
				return nResult > 0;
			}
			return vRNode.GetIndex<IDX_NAME>().Compare(lhs[2].first, lhs[2].second) > 0;
		}
	};

private:
	// http://tools.ietf.org/html/rfc6265
	static bool xIsCookieNodeValid(const xCookieMap::Node &vCookieNode) noexcept {
		if(vCookieNode.GetIndex<IDX_NAME>().IsEmpty()){
			return false;
		}
		if(vCookieNode.GetElement().IsEmpty()){
			return false;
		}
		if(vCookieNode.GetIndex<IDX_PATH>().IsEmpty()){
			return false;
		}
		if(vCookieNode.GetIndex<IDX_DOMAIN>().IsEmpty()){
			return false;
		}
		return true;
	}
	static xCookieMap::Node xCookieFromHeader(
		const char *pchBegin,
		const char *pchEnd,
		std::uint64_t uCurrentTime,
		const Utf8String &u8sRequestPath,
		const Utf8String &u8sRevHostName
	){
		const auto pchNameValueEnd = std::find(pchBegin, pchEnd, ';');
		if(pchBegin == pchNameValueEnd){
			return NULL_COOKIE_NODE;
		}
		const auto pchEqualSignPos = std::find(pchBegin, pchNameValueEnd, '=');
		if((pchBegin == pchEqualSignPos) || (pchEqualSignPos == pchNameValueEnd) || (pchEqualSignPos + 1 == pchNameValueEnd)){
			return NULL_COOKIE_NODE;
		}

		// http://tools.ietf.org/html/rfc2616#section-2.2
		static constexpr std::uint32_t TABLE[8] = {
			0x00000000,	// 0x00 - 0x1F
			0x03FF6C7E,	// 0x20 - 0x3F
			0xC7FFFFFE,	// 0x40 - 0x5F
			0x57FFFFFF,	// 0x60 - 0x7F
			0x00000000,	// 0x80 - 0x9F
			0x00000000,	// 0xA0 - 0xBF
			0x00000000,	// 0xC0 - 0xDF
			0x00000000	// 0xE0 - 0xFF
		};
		auto pchCur = pchBegin;
		while(pchCur != pchEqualSignPos){
			const unsigned char uch = *(pchCur++);
			if(!(TABLE[uch >> 5] & (1u << (uch & 0x1F)))){
				return NULL_COOKIE_NODE;
			}
		}
		if(pchCur != pchEqualSignPos){
			return NULL_COOKIE_NODE;
		}

		Utf8String u8sName(pchBegin, pchEqualSignPos);
		Utf8String u8sValue(pchEqualSignPos + 1, pchNameValueEnd);

		std::uint64_t u64Expires = 0;
		std::uint64_t u64ExpiresFromMaxAge = 0;
		Utf8String u8sPath;
		Utf8String u8sDomain;
		bool bSecure = false;
		bool bHttpOnly = false;

		auto pchPartBegin = pchNameValueEnd;
		while((pchPartBegin != pchEnd) && (++pchPartBegin != pchEnd) && (*pchPartBegin == ' ') && (++pchPartBegin != pchEnd)){
			const auto pchPartEnd = std::find(pchPartBegin, pchEnd, ';');

			const auto pchEqualSignPos = std::find(pchPartBegin, pchPartEnd, '=');
			const auto pchAttrValBegin = (pchEqualSignPos == pchPartEnd) ? pchPartEnd : (pchEqualSignPos + 1);

			const auto CheckAttribute = [=](const char *pwszAttrName) noexcept {
				return std::equal(
					pchPartBegin,
					pchEqualSignPos,
					pwszAttrName,
					[](char lhs, char rhs) noexcept { return ((lhs ^ rhs) & ~0x20) == 0; }
				);
			};

			switch(pchEqualSignPos - pchPartBegin){
			case 8:
				if(CheckAttribute("httponly")){
					bHttpOnly = true;
				}
				break;

			case 7:
				if(CheckAttribute("expires")){
					Utf16String wcsHttpTime(Utf8String(pchAttrValBegin, pchPartEnd));
					SYSTEMTIME vSystemTime;
					if(::WinHttpTimeToSystemTime(wcsHttpTime.GetCStr(), &vSystemTime)){
						::SystemTimeToFileTime(&vSystemTime, (FILETIME *)&u64Expires);
					}
				} else if(CheckAttribute("max-age")){
					u64ExpiresFromMaxAge = uCurrentTime + std::strtoull(pchAttrValBegin, nullptr, 10) * 10000000u;
				}
				break;

			case 6:
				if(CheckAttribute("domain")){
					if((pchAttrValBegin != pchPartEnd) && (pchPartEnd[-1] != '.')){
						auto pchDomainBegin = pchAttrValBegin;
						if(*pchDomainBegin == '.'){
							++pchDomainBegin;
						}
						u8sDomain.Resize(pchPartEnd - pchDomainBegin + 1);
						auto ritWrite = std::reverse_iterator<char *>(u8sDomain.GetEnd());
						*ritWrite = '.';
						++ritWrite;
						std::transform(
							pchDomainBegin,
							pchPartEnd,
							ritWrite,
							[](char ch) noexcept { return (('A' <= ch) && (ch <= 'Z')) ? (ch | 0x20) : ch; }
						);
					}
				} else if(CheckAttribute("secure")){
					bSecure = true;
				}
				break;

			case 4:
				if(CheckAttribute("path")){
					if((pchAttrValBegin != pchPartEnd) && (pchAttrValBegin[0] == '/')){
						u8sPath.Assign(pchAttrValBegin, pchPartEnd);
					}
				}
				break;
			}

			pchPartBegin = pchPartEnd;
		}

		if(u8sPath.IsEmpty()){
			ASSERT(u8sRequestPath[0] == '/');

			u8sPath = u8sRequestPath;
		}

		const auto pchRevHostBegin = u8sRevHostName.GetBegin();
		const auto pchRevHostEnd = u8sRevHostName.GetEnd();
		if(u8sDomain.IsEmpty() || xIsIP(pchRevHostBegin, pchRevHostEnd)){
			u8sDomain.Resize(pchRevHostEnd - pchRevHostBegin + 1);
			u8sDomain.Assign(pchRevHostBegin, pchRevHostEnd);
			u8sDomain.Push('$');
		} else {
			if(!xCanHostMatchDomain(u8sRevHostName, u8sDomain)){
				return NULL_COOKIE_NODE;
			}
		}

		xCookieMap::Node vNodeRet(
			std::move(u8sValue),
			std::move(u8sName),
			(u64ExpiresFromMaxAge != 0) ? u64ExpiresFromMaxAge : u64Expires,
			std::move(u8sPath),
			std::move(u8sDomain),
			bSecure,
			bHttpOnly,
			0
		);
		return std::move(vNodeRet);
	}

	static bool xIsIP(const char *pchRevHostBegin, const char *pchRevHostEnd) noexcept {
		if(std::find(pchRevHostBegin, pchRevHostEnd, ':') != pchRevHostEnd){ // IPv6
			return true;
		}
		if(std::find_if_not(
			pchRevHostBegin,
			pchRevHostEnd,
			[](char ch){ return (ch == '.') || (('0' <= ch) && (ch <= '9')); }
		) == pchRevHostEnd){ // IPv4
			return true;
		}
		return false;
	}
	static bool xCanHostMatchDomain(const Utf8String &u8sRevHostName, const Utf8String &u8sDomain) noexcept {
		const auto uHostLen = u8sRevHostName.GetSize();
		if(uHostLen == 0){
			return false;
		}
		const auto uQualifiedDomainLen = u8sDomain.GetSize();
		if(uQualifiedDomainLen <= 1u){
			return false;
		}
		const auto uDomainLen = uQualifiedDomainLen - 1;

		const auto pchRevHostBegin = u8sRevHostName.GetCStr();
		const auto pchRevHostEnd = pchRevHostBegin + uHostLen;

		switch(u8sDomain[uDomainLen]){
		case '$':
			return (uHostLen == uDomainLen) && std::equal(
				pchRevHostBegin,
				pchRevHostEnd,
				u8sDomain.GetCStr(),
				[](char lhs, char rhs) noexcept { return ((lhs ^ rhs) & ~0x20) == 0; }
			);

		case '.':
			if(uHostLen < uDomainLen){
				return false;
			} else if((uHostLen > uDomainLen) && (pchRevHostBegin[uDomainLen] != '.')){
				return false;
			}
			if(!std::equal(
				pchRevHostBegin,
				pchRevHostBegin + uDomainLen,
				u8sDomain.GetCStr(),
				[](char lhs, char rhs) noexcept { return ((lhs ^ rhs) & ~0x20) == 0; }
			)){
				return false;
			}
			if((uHostLen > uDomainLen) && xIsIP(pchRevHostBegin, pchRevHostEnd)){
				return false;
			}
			return true;

		default:
			return false;
		}
	}
	static bool xCanRequestPathMatchPath(const Utf8String &u8sRequestPath, const Utf8String &u8sPath) noexcept {
		const auto uRequestPathLen = u8sRequestPath.GetSize();
		if(uRequestPathLen == 0){
			return false;
		}
		const auto uPathLen = u8sPath.GetSize();
		if(uPathLen == 0){
			return false;
		}

		if(uRequestPathLen < uPathLen){
			return false;
		}
		if(u8sRequestPath.Compare(u8sPath, uPathLen) == 0){
			if(uRequestPathLen == uPathLen){
				return true;
			}
			if(u8sPath[uPathLen - 1] == '/'){
				return true;
			}
			if(u8sRequestPath[uPathLen] == '/'){
				return true;
			}
		}
		return false;
	}

private:
	Utf16String xm_wcsPacUrl;
	Utf16String xm_wcsProxyUserName;
	Utf16String xm_wcsProxyPassword;
	xWinHttpHandle xm_hSession;

	xSharedWinHttpHandle xm_hConnect;
	Utf16String xm_wcsLastHost;
	int xm_nLastScheme;

	xWinHttpHandle xm_hRequest;

	xCookieMap xm_mapCookies;

public:
	xDelegate(bool bAutoProxy, const wchar_t *pwchProxy, std::size_t uProxyLen, const wchar_t *pwszUserAgent)
		: xm_nLastScheme(0)
	{
		if(!pwchProxy){
			pwchProxy = L"";
			uProxyLen = 0;
		} else if(uProxyLen == (std::size_t)-1){
			uProxyLen = StrLen(pwchProxy);
		}

		Utf16String ucsNamedProxy;
		Utf16String ucsNamedProxyBypass;

		if(uProxyLen > 0){
			if(bAutoProxy){
				// 使用指定的 PAC（指定 URL）
				xm_wcsPacUrl = pwchProxy;
			} else {
				// 使用指定的代理服务器（user:pass@addr:port|bypass）
				const auto pwchBypassBegin = std::wmemchr(pwchProxy, L'|', uProxyLen);
				const auto uProxyUrlLen = pwchBypassBegin ? (std::size_t)(pwchBypassBegin - pwchProxy) : uProxyLen;

				URL_COMPONENTS vUrlComponents;
				vUrlComponents.dwStructSize			= sizeof(vUrlComponents);
				vUrlComponents.lpszScheme			= nullptr;
				vUrlComponents.dwSchemeLength		= 0;
				vUrlComponents.lpszHostName			= nullptr;
				vUrlComponents.dwHostNameLength		= 1;
				vUrlComponents.lpszUserName			= nullptr;
				vUrlComponents.dwUserNameLength		= 1;
				vUrlComponents.lpszPassword			= nullptr;
				vUrlComponents.dwPasswordLength		= 1;
				vUrlComponents.lpszUrlPath			= nullptr;
				vUrlComponents.dwUrlPathLength		= 0;
				vUrlComponents.lpszExtraInfo		= nullptr;
				vUrlComponents.dwExtraInfoLength	= 0;
				if(!::WinHttpCrackUrl(pwchProxy, uProxyUrlLen, 0, &vUrlComponents)){
					MCF_THROW(::GetLastError(), L"::WinHttpCrackUrl() 失败。");
				}

				if(vUrlComponents.lpszUserName){
					xm_wcsProxyUserName.Assign(vUrlComponents.lpszUserName, vUrlComponents.dwUserNameLength);
					vUrlComponents.lpszUserName = nullptr;
					vUrlComponents.dwUserNameLength = 0;
				}
				if(vUrlComponents.lpszPassword){
					xm_wcsProxyPassword.Assign(vUrlComponents.lpszPassword, vUrlComponents.dwPasswordLength);
					vUrlComponents.lpszPassword = nullptr;
					vUrlComponents.dwPasswordLength = 0;
				}

				DWORD dwUrlLength = 0;
				::WinHttpCreateUrl(&vUrlComponents, 0, nullptr, &dwUrlLength);
				const auto ulErrorCode = ::GetLastError();
				if(ulErrorCode != ERROR_INSUFFICIENT_BUFFER){
					MCF_THROW(::GetLastError(), L"::WinHttpCreateUrl() 失败。");
				}
				ucsNamedProxy.Resize(dwUrlLength);
				if(!::WinHttpCreateUrl(&vUrlComponents, 0, ucsNamedProxy.GetCStr(), &dwUrlLength)){
					MCF_THROW(::GetLastError(), L"::WinHttpCreateUrl() 失败。");
				}
				ucsNamedProxy.Resize(dwUrlLength);

				if(pwchBypassBegin){
					ucsNamedProxyBypass.Assign(pwchBypassBegin + 1, pwchProxy + uProxyLen);
				}
			}
		} else if(bAutoProxy){
			// 使用 IE 的代理服务器
			WINHTTP_CURRENT_USER_IE_PROXY_CONFIG vIEProxyConfig;
			if(::WinHttpGetIEProxyConfigForCurrentUser(&vIEProxyConfig)){
				const xGlobalMem pAutoConfigUrl	(vIEProxyConfig.lpszAutoConfigUrl);
				const xGlobalMem pProxy			(vIEProxyConfig.lpszProxy);
				const xGlobalMem pProxyBypass		(vIEProxyConfig.lpszProxyBypass);

				if(vIEProxyConfig.fAutoDetect){
					xm_wcsPacUrl = vIEProxyConfig.lpszAutoConfigUrl;
				} else {
					ucsNamedProxy = vIEProxyConfig.lpszProxy;
					ucsNamedProxyBypass = vIEProxyConfig.lpszProxyBypass;
				}
			}
		}
		xm_hSession.Reset(::WinHttpOpen(
			pwszUserAgent,
			ucsNamedProxy.IsEmpty() ? WINHTTP_ACCESS_TYPE_NO_PROXY : WINHTTP_ACCESS_TYPE_NAMED_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0
		));
		if(!xm_hSession){
			MCF_THROW(::GetLastError(), L"::WinHttpOpen() 失败。");
		}
	}

private:
	void xUpdateCookie(xCookieMap::Node &&vNode){
		const auto pOldNode = xm_mapCookies.Find<IDX_DOMAIN_PATH_NAME>(vNode.GetIndex<IDX_DOMAIN_PATH_NAME>());
		xm_mapCookies.Insert(std::move(vNode));
		if(pOldNode){
			xm_mapCookies.Erase(pOldNode);
		}
	}
	void xTidyExpiredCookies(std::uint64_t u64Time = GetNtTime()) noexcept {
		auto pNode = xm_mapCookies.GetUpperBound<IDX_EXPIRES>(0);
		while(pNode && (pNode->GetIndex<IDX_EXPIRES>() < u64Time)){
			const auto pNext = xm_mapCookies.GetNext<IDX_EXPIRES>(pNode);
			xm_mapCookies.Erase(pNode);
			pNode = pNext;
		}
	}

public:
	bool AddCookie(
		Utf8String u8sName,
		const Utf8String &u8sValue,
		std::uint64_t u64Expires,
		Utf8String u8sPath,
		Utf8String u8sDomain,
		bool bSecure,
		bool bHttpOnly
	){
		if(u8sName.IsEmpty()){
			return false;
		}
		const auto uDomainLen = u8sDomain.GetSize();
		if(uDomainLen == 0){
			return false;
		}

		if(u8sPath[0] != '/'){
			u8sPath.Unshift('/');
		}
		switch(u8sDomain[uDomainLen - 1]){
		case '.':
		case '$':
			break;

		default:
			u8sDomain.Push('.');
			break;
		}
		u8sDomain.Reverse();

		if(u8sValue.IsEmpty()){
			const std::pair<const char *, std::size_t> aDomainPathName[3] = {
				u8sDomain.GetCStrLength(),
				u8sPath.GetCStrLength(),
				u8sName.GetCStrLength()
			};
			RemoveCookie(aDomainPathName);
		} else {
			Utf8String u8sEncodedValue;
			UrlEncode(u8sEncodedValue, u8sValue);

			xCookieMap::Node vNode(
				std::move(u8sEncodedValue),
				std::move(u8sName),
				u64Expires,
				std::move(u8sPath),
				std::move(u8sDomain),
				bSecure,
				bHttpOnly,
				0
			);
			xUpdateCookie(std::move(vNode));
		}
		return true;
	}
	bool RemoveCookie(const std::pair<const char *, std::size_t> (&aNamePathDomain)[3]) noexcept {
		auto pNode = xm_mapCookies.Find<IDX_DOMAIN_PATH_NAME>(aNamePathDomain);
		if(!pNode){
			return false;
		}
		xm_mapCookies.Erase(pNode);
		return true;
	}
	void ClearCookies() noexcept {
		xm_mapCookies.Clear();
	}

	Vector<Vector<unsigned char>> ExportCookies(bool bIncludeSessionOnly){
		xTidyExpiredCookies();

		Vector<Vector<unsigned char>> vecRet;
		auto pNode = bIncludeSessionOnly ? xm_mapCookies.GetBegin<IDX_EXPIRES>() : xm_mapCookies.GetUpperBound<IDX_EXPIRES>(0);
		while(pNode){
			vecRet.Push();
			auto &vecDst = vecRet.GetEnd()[-1];
			vecDst.Reserve(192);

			unsigned char abyTempBuffer[9];
			VarIntEx<std::uint64_t> viTemp;
			unsigned char byTemp;

			viTemp = pNode->GetIndex<IDX_NAME>().GetLength();
			vecDst.CopyToEnd(abyTempBuffer, viTemp.Serialize(abyTempBuffer));
			vecDst.CopyToEnd((const unsigned char *)pNode->GetIndex<IDX_NAME>().GetCStr(), viTemp.Get());

			viTemp = pNode->GetElement().GetLength();
			vecDst.CopyToEnd(abyTempBuffer, viTemp.Serialize(abyTempBuffer));
			vecDst.CopyToEnd((const unsigned char *)pNode->GetElement().GetCStr(), viTemp.Get());

			viTemp = pNode->GetIndex<IDX_EXPIRES>() / 10000000u;
			vecDst.CopyToEnd(abyTempBuffer, viTemp.Serialize(abyTempBuffer));

			viTemp = pNode->GetIndex<IDX_PATH>().GetLength();
			vecDst.CopyToEnd(abyTempBuffer, viTemp.Serialize(abyTempBuffer));
			vecDst.CopyToEnd((const unsigned char *)pNode->GetIndex<IDX_PATH>().GetCStr(), viTemp.Get());

			viTemp = pNode->GetIndex<IDX_DOMAIN>().GetLength();
			vecDst.CopyToEnd(abyTempBuffer, viTemp.Serialize(abyTempBuffer));
			vecDst.CopyToEnd((const unsigned char *)pNode->GetIndex<IDX_DOMAIN>().GetCStr(), viTemp.Get());

			byTemp = pNode->GetIndex<IDX_SECURE>();
			vecDst.CopyToEnd(&byTemp, sizeof(byTemp));

			byTemp = pNode->GetIndex<IDX_HTTP_ONLY>();
			vecDst.CopyToEnd(&byTemp, sizeof(byTemp));

			pNode = xm_mapCookies.GetNext<IDX_EXPIRES>(pNode);
		}
		return std::move(vecRet);
	}
	void ImportCookies(const Vector<Vector<unsigned char>> &vecData){
		Utf8String u8sDecodedValue;
		for(auto &vecSerialized : vecData){
			auto pchBegin = vecSerialized.GetBegin();
			const auto pchEnd = vecSerialized.GetEnd();

			VarIntEx<std::uint64_t> viTemp;

			pchBegin = viTemp.Unserialize(pchBegin, pchEnd);
			if(!pchBegin){
				continue;
			}
			if(viTemp.Get() == 0){
				continue;
			}
			if((std::size_t)(pchEnd - pchBegin) < viTemp.Get()){
				continue;
			}
			Utf8String u8sName((const char *)pchBegin, viTemp.Get());
			pchBegin += viTemp.Get();

			pchBegin = viTemp.Unserialize(pchBegin, pchEnd);
			if(!pchBegin){
				continue;
			}
			if(viTemp.Get() == 0){
				continue;
			}
			if((std::size_t)(pchEnd - pchBegin) < viTemp.Get()){
				continue;
			}
			Utf8String u8sDecodedValue((const char *)pchBegin, viTemp.Get());
			pchBegin += viTemp.Get();

			pchBegin = viTemp.Unserialize(pchBegin, pchEnd);
			if(!pchBegin){
				continue;
			}
			const std::uint64_t uExpires = viTemp * 10000000u;

			pchBegin = viTemp.Unserialize(pchBegin, pchEnd);
			if(!pchBegin){
				continue;
			}
			if((std::size_t)(pchEnd - pchBegin) < viTemp.Get()){
				continue;
			}
			Utf8String u8sPath((const char *)pchBegin, viTemp.Get());
			pchBegin += viTemp.Get();

			pchBegin = viTemp.Unserialize(pchBegin, pchEnd);
			if(!pchBegin){
				continue;
			}
			if((std::size_t)(pchEnd - pchBegin) < viTemp.Get()){
				continue;
			}
			Utf8String u8sDomain((const char *)pchBegin, viTemp.Get());
			pchBegin += viTemp.Get();

			if(pchBegin == pchEnd){
				continue;
			}
			const bool bSecure = *(pchBegin++);

			if(pchBegin == pchEnd){
				continue;
			}
			const bool bHttpOnly = *(pchBegin++);

			if(pchBegin != pchEnd){
				continue;
			}
			xUpdateCookie(xCookieMap::Node(
				std::move(u8sDecodedValue),
				std::move(u8sName),
				uExpires,
				std::move(u8sPath),
				std::move(u8sDomain),
				bSecure,
				bHttpOnly,
				0
			));
		}
	}

	void Connect(
		const wchar_t *pwszVerb,
		const wchar_t *pwchUrl,
		std::size_t uUrlLen,
		const void *pContents,
		std::size_t uContentSize,
		const wchar_t *pwszContentType
	){
		if(uUrlLen == (std::size_t)-1){
			uUrlLen = StrLen(pwchUrl);
		}

		URL_COMPONENTS vUrlComponents;
		vUrlComponents.dwStructSize			= sizeof(vUrlComponents);
		vUrlComponents.lpszScheme			= nullptr;
		vUrlComponents.dwSchemeLength		= 1;
		vUrlComponents.lpszHostName			= nullptr;
		vUrlComponents.dwHostNameLength		= 1;
		vUrlComponents.lpszUserName			= nullptr;
		vUrlComponents.dwUserNameLength		= 1;
		vUrlComponents.lpszPassword			= nullptr;
		vUrlComponents.dwPasswordLength		= 1;
		vUrlComponents.lpszUrlPath			= nullptr;
		vUrlComponents.dwUrlPathLength		= 1;
		vUrlComponents.lpszExtraInfo		= nullptr;
		vUrlComponents.dwExtraInfoLength	= 1;
		if(!::WinHttpCrackUrl(pwchUrl, uUrlLen, 0, &vUrlComponents)){
			MCF_THROW(::GetLastError(), L"::WinHttpCrackUrl() 失败。");
		}

		Utf16String wcsHostName;
		wcsHostName.Resize(vUrlComponents.dwHostNameLength);
		std::transform(
			vUrlComponents.lpszHostName,
			vUrlComponents.lpszHostName + vUrlComponents.dwHostNameLength,
			wcsHostName.GetCStr(),
			[](wchar_t wch) noexcept { return ((L'A' <= wch) && (wch <= L'Z')) ? (wch & -0x20) : wch; }
		);

		Utf16String wcsPath(vUrlComponents.lpszUrlPath, vUrlComponents.dwUrlPathLength);
		if(wcsPath[0] != L'/'){
			wcsPath = L'/';
		}

		Utf16String wcsPathExtra(wcsPath);
		wcsPathExtra.Append(vUrlComponents.lpszExtraInfo, vUrlComponents.dwExtraInfoLength);

		Utf16String wcsUserName;
		Utf16String wcsPassword;

		if(vUrlComponents.lpszUserName){
			wcsUserName.Assign(vUrlComponents.lpszUserName, vUrlComponents.dwUserNameLength);
			vUrlComponents.lpszUserName = nullptr;
			vUrlComponents.dwUserNameLength = 0;
		}
		if(vUrlComponents.lpszPassword){
			wcsPassword.Assign(vUrlComponents.lpszPassword, vUrlComponents.dwPasswordLength);
			vUrlComponents.lpszPassword = nullptr;
			vUrlComponents.dwPasswordLength = 0;
		}

		xSharedWinHttpHandle hConnect;
		if((xm_wcsLastHost == wcsHostName) && (xm_nLastScheme == vUrlComponents.nScheme)){
			hConnect = xm_hConnect;
		} else {
			hConnect.Reset(::WinHttpConnect(
				xm_hSession.Get(),
				wcsHostName.GetCStr(),
				vUrlComponents.nPort,
				0
			));
		}
		if(!hConnect){
			MCF_THROW(::GetLastError(), L"::WinHttpConnect() 失败。");
		}

		xWinHttpHandle hRequest(::WinHttpOpenRequest(
			hConnect.Get(),
			pwszVerb,
			wcsPathExtra.GetCStr(),
			nullptr,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			(vUrlComponents.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0
		));
		if(!hRequest){
			MCF_THROW(::GetLastError(), L"::WinHttpOpenRequest() 失败。");
		}

		static const DWORD DISABLE_FLAGS = WINHTTP_DISABLE_COOKIES | WINHTTP_DISABLE_REDIRECTS ;
		if(!::WinHttpSetOption(hRequest.Get(), WINHTTP_OPTION_DISABLE_FEATURE, (void *)&DISABLE_FLAGS, sizeof(DISABLE_FLAGS))){
			MCF_THROW(::GetLastError(), L"::WinHttpSetOption() 失败。");
		}

		if(!xm_wcsPacUrl.IsEmpty()){
			DWORD dwUrlLength = 0;
			::WinHttpCreateUrl(&vUrlComponents, 0, nullptr, &dwUrlLength);
			const auto ulErrorCode = ::GetLastError();
			if(ulErrorCode != ERROR_INSUFFICIENT_BUFFER){
				MCF_THROW(::GetLastError(), L"::WinHttpCreateUrl() 失败。");
			}
			Utf16String wcsUrlWithoutAuth;
			wcsUrlWithoutAuth.Resize(dwUrlLength);
			if(!::WinHttpCreateUrl(&vUrlComponents, 0, wcsUrlWithoutAuth.GetCStr(), &dwUrlLength)){
				MCF_THROW(::GetLastError(), L"::WinHttpCreateUrl() 失败。");
			}
			wcsUrlWithoutAuth.Resize(dwUrlLength);

			WINHTTP_AUTOPROXY_OPTIONS vAutoProxyOptions;
			vAutoProxyOptions.dwFlags					= WINHTTP_AUTOPROXY_CONFIG_URL;
			vAutoProxyOptions.dwAutoDetectFlags			= 0;
			vAutoProxyOptions.lpszAutoConfigUrl			= xm_wcsPacUrl.GetCStr();
			vAutoProxyOptions.lpvReserved 				= nullptr;
			vAutoProxyOptions.dwReserved  				= 0;
			vAutoProxyOptions.fAutoLogonIfChallenged	= FALSE;

			WINHTTP_PROXY_INFO vProxyInfo;
			if(!::WinHttpGetProxyForUrl(xm_hSession.Get(), wcsUrlWithoutAuth.GetCStr(), &vAutoProxyOptions, &vProxyInfo)){
				MCF_THROW(::GetLastError(), L"::WinHttpGetProxyForUrl() 失败。");
			}
			const xGlobalMem pProxy		(vProxyInfo.lpszProxy);
			const xGlobalMem pProxyBypass	(vProxyInfo.lpszProxyBypass);

			if(!::WinHttpSetOption(hRequest.Get(), WINHTTP_OPTION_PROXY, &vProxyInfo, sizeof(vProxyInfo))){
				MCF_THROW(::GetLastError(), L"::WinHttpSetOption() 失败。");
			}
		}

		if(!xm_wcsProxyUserName.IsEmpty()){
			if(!::WinHttpSetCredentials(
				hRequest.Get(),
				WINHTTP_AUTH_TARGET_PROXY,
				WINHTTP_AUTH_SCHEME_BASIC,
				xm_wcsProxyUserName.GetCStr(),
				xm_wcsProxyPassword.GetCStr(),
				nullptr
			)){
				MCF_THROW(::GetLastError(), L"::WinHttpSetCredentials() 失败。");
			}
		}

		if(!wcsUserName.IsEmpty()){
			if(!::WinHttpSetCredentials(
				hRequest.Get(),
				WINHTTP_AUTH_TARGET_SERVER,
				WINHTTP_AUTH_SCHEME_BASIC,
				wcsUserName.GetCStr(),
				wcsPassword.GetCStr(),
				nullptr
			)){
				MCF_THROW(::GetLastError(), L"::WinHttpSetCredentials() 失败。");
			}
		}

		const auto u64NtTime = GetNtTime();
		Utf8String u8sRevHostName(wcsHostName);
		u8sRevHostName.Reverse();

		Utf8String u8sRequestPath(wcsPath);
		if(u8sRequestPath[0] != '/'){
			u8sRequestPath = '/';
		}

		xTidyExpiredCookies(u64NtTime);

		Utf16String wcsCookie(L"Cookie: ");
		auto pNext = xm_mapCookies.GetBegin<IDX_DOMAIN>();
		while(pNext){
			const auto pNode = pNext;
			pNext = xm_mapCookies.GetNext<IDX_DOMAIN>(pNext);

			if(pNode->GetIndex<IDX_HTTP_ONLY>() && (vUrlComponents.nScheme != INTERNET_SCHEME_HTTPS)){
				continue;
			}
			if(!xCanHostMatchDomain(u8sRevHostName, pNode->GetIndex<IDX_DOMAIN>())){
				continue;
			}
			if(!xCanRequestPathMatchPath(u8sRequestPath, pNode->GetIndex<IDX_PATH>())){
				continue;
			}

			wcsCookie += pNode->GetIndex<IDX_NAME>();
			wcsCookie += L'=';
			wcsCookie += pNode->GetElement();
			wcsCookie += L';';
			wcsCookie += L' ';
		}
		if(wcsCookie.GetSize() > 8u){ // wcslen(L"Cookie: ")
			wcsCookie.Pop(2);
			if(!::WinHttpAddRequestHeaders(
				hRequest.Get(),
				wcsCookie.GetCStr(),
				wcsCookie.GetSize(),
				WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE
			)){
				MCF_THROW(::GetLastError(), L"::WinHttpAddRequestHeaders() 失败。");
			}
		}

		if(uContentSize > 0){
			Utf16String wcsContentType(L"Content-Type: ");
			wcsContentType += pwszContentType;
			if(!::WinHttpAddRequestHeaders(
				hRequest.Get(),
				wcsContentType.GetCStr(),
				wcsContentType.GetSize(),
				WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE
			)){
				MCF_THROW(::GetLastError(), L"::WinHttpAddRequestHeaders() 失败。");
			}
		}

		if(!::WinHttpSendRequest(
			hRequest.Get(),
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			(void *)pContents,
			uContentSize,
			uContentSize,
			0
		)){
			MCF_THROW(::GetLastError(), L"::WinHttpSendRequest() 失败。");
		}

		if(!::WinHttpReceiveResponse(hRequest.Get(), nullptr)){
			MCF_THROW(::GetLastError(), L"::WinHttpReceiveResponse() 失败。");
		}

		Utf8String u8sSetCookie;
		std::size_t uSetCookieIndex = 0;
		for(;;){
			Utf16String wcsSetCookie;
			DWORD dwSetCookieLength = 127 * sizeof(wchar_t);
			wcsSetCookie.Resize(dwSetCookieLength / sizeof(wchar_t));
			DWORD dwSetCookieIndex = uSetCookieIndex;
			if(!::WinHttpQueryHeaders(
				hRequest.Get(),
				WINHTTP_QUERY_SET_COOKIE,
				WINHTTP_HEADER_NAME_BY_INDEX,
				wcsSetCookie.GetCStr(),
				&dwSetCookieLength,
				&dwSetCookieIndex
			)){
				const auto ulErrorCode = ::GetLastError();
				if(ulErrorCode == ERROR_WINHTTP_HEADER_NOT_FOUND){
					break;
				}
				if(ulErrorCode != ERROR_INSUFFICIENT_BUFFER){
					MCF_THROW(ulErrorCode, L"::WinHttpQueryHeaders() 失败。");
				}
				wcsSetCookie.Resize(dwSetCookieLength / sizeof(wchar_t));
				dwSetCookieIndex = uSetCookieIndex;
				if(!::WinHttpQueryHeaders(
					hRequest.Get(),
					WINHTTP_QUERY_SET_COOKIE,
					WINHTTP_HEADER_NAME_BY_INDEX,
					wcsSetCookie.GetCStr(),
					&dwSetCookieLength,
					&dwSetCookieIndex
				)){
					MCF_THROW(::GetLastError(), L"::WinHttpQueryHeaders() 失败。");
				}
			}
			wcsSetCookie.Resize(dwSetCookieLength / sizeof(wchar_t));
			++uSetCookieIndex;

			u8sSetCookie = wcsSetCookie;
			auto vCookieNode = xCookieFromHeader(u8sSetCookie.GetBegin(), u8sSetCookie.GetEnd(), u64NtTime, u8sRequestPath, u8sRevHostName);
			if(xIsCookieNodeValid(vCookieNode)){
				xUpdateCookie(std::move(vCookieNode));
			}
		}

		char buffer[4096];
		DWORD read;
		::WinHttpReadData(hRequest.Get(), buffer, sizeof(buffer) - 1, &read);
		buffer[read] = 0;
		std::puts(AnsiString(Utf8String(buffer, read)).GetCStr());

		xm_hConnect		= std::move(hConnect);
		xm_wcsLastHost	= std::move(wcsHostName);
		xm_nLastScheme	= vUrlComponents.nScheme;

		xm_hRequest		= std::move(hRequest);
	}
	void Disconnect() noexcept {
		xm_hRequest.Reset();
		xm_hConnect.Reset();
	}
};

const HttpClient::xDelegate::xCookieMap::Node HttpClient::xDelegate::NULL_COOKIE_NODE(Utf8String(), Utf8String(), 0, Utf8String(), Utf8String(), 0, 0, 0);

// 构造函数和析构函数。
HttpClient::HttpClient(bool bAutoProxy, const wchar_t *pwchProxy, std::size_t uProxyLen, const wchar_t *pwszUserAgent)
	: xm_pDelegate(new xDelegate(bAutoProxy, pwchProxy, uProxyLen, pwszUserAgent))
{
}
HttpClient::HttpClient(bool bAutoProxy, const Utf16String &wcsProxy, const wchar_t *pwszUserAgent)
	: HttpClient(bAutoProxy, wcsProxy.GetCStr(), wcsProxy.GetLength(), pwszUserAgent)
{
}
HttpClient::HttpClient(HttpClient &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
HttpClient &HttpClient::operator=(HttpClient &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
HttpClient::~HttpClient(){
}

// 其他非静态成员函数。
bool HttpClient::AddCookie(
	Utf8String			u8sName,
	const Utf8String &	u8sValue,
	std::uint64_t		u64Expires,
	Utf8String			u8sPath,
	Utf8String			u8sDomain,
	bool				bSecure,
	bool				bHttpOnly
){
	return xm_pDelegate->AddCookie(
		std::move(u8sName),
		u8sValue,
		u64Expires,
		std::move(u8sPath),
		std::move(u8sDomain),
		bSecure,
		bHttpOnly
	);
}
bool HttpClient::RemoveCookie(
	const std::pair<const char *, std::size_t> &vName,
	const std::pair<const char *, std::size_t> &vPath,
	const std::pair<const char *, std::size_t> &vDomain
) noexcept {
	return xm_pDelegate->RemoveCookie({vDomain, vPath, vName});
}
bool HttpClient::RemoveCookie(
	const Utf8String &u8sName,
	const Utf8String &u8sPath,
	const Utf8String &u8sDomain
) noexcept {
	return RemoveCookie(u8sName.GetCStrLength(), u8sPath.GetCStrLength(), u8sDomain.GetCStrLength());
}
void HttpClient::ClearCookies() noexcept {
	xm_pDelegate->ClearCookies();
}

unsigned long HttpClient::ConnectNoThrow(
	const wchar_t *	pwszVerb,
	const wchar_t *	pwchUrl,
	std::size_t		uUrlLen,
	const void *	pContents,
	std::size_t		uContentSize,
	const wchar_t *	pwszContentType
){
	try {
		Connect(pwszVerb, pwchUrl, uUrlLen, pContents, uContentSize, pwszContentType);
		return ERROR_SUCCESS;
	} catch(Exception &e){
		return e.ulErrorCode;
	}
}
void HttpClient::Connect(
	const wchar_t *	pwszVerb,
	const wchar_t *	pwchUrl,
	std::size_t		uUrlLen,
	const void *	pContents,
	std::size_t		uContentSize,
	const wchar_t *	pwszContentType
){
	xm_pDelegate->Connect(pwszVerb, pwchUrl, uUrlLen, pContents, uContentSize, pwszContentType);
}

unsigned long HttpClient::ConnectNoThrow(
	const wchar_t *		pwszVerb,
	const Utf16String &	wcsUrl,
	const void *		pContents,
	std::size_t			uContentSize,
	const wchar_t *		pwszContentType
){
	return ConnectNoThrow(pwszVerb, wcsUrl.GetCStr(), wcsUrl.GetLength(), pContents, uContentSize, pwszContentType);
}
void HttpClient::Connect(
	const wchar_t *		pwszVerb,
	const Utf16String &	wcsUrl,
	const void *		pContents,
	std::size_t			uContentSize,
	const wchar_t *		pwszContentType
){
	Connect(pwszVerb, wcsUrl.GetCStr(), wcsUrl.GetLength(), pContents, uContentSize, pwszContentType);
}
void HttpClient::Disconnect() noexcept {
	xm_pDelegate->Disconnect();
}

Vector<Vector<unsigned char>> HttpClient::ExportCookies(bool bIncludeSessionOnly) const {
	return xm_pDelegate->ExportCookies(bIncludeSessionOnly);
}
void HttpClient::ImportCookies(const Vector<Vector<unsigned char>> &vecData){
	xm_pDelegate->ImportCookies(vecData);
}
*/
