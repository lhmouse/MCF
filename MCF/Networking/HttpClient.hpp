// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_HTTP_CLIENT_HPP__
#define __MCF_HTTP_CLIENT_HPP__

#include "../../MCFCRT/cpp/ext/vvector.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/String.hpp"
#include "PeerInfo.hpp"
#include <memory>
#include <functional>
#include <cstddef>
#include <cstdint>

namespace MCF {

// http://tools.ietf.org/html/rfc3986
extern void UrlEncode(Utf8String &u8sAppendTo, const char *pchBegin, std::size_t uLen = (std::size_t)-1);
extern void UrlEncode(Utf8String &u8sAppendTo, const Utf8String &u8sSrc);
extern void UrlDecode(Utf8String &u8sAppendTo, const char *pchBegin, std::size_t uLen = (std::size_t)-1);
extern void UrlDecode(Utf8String &u8sAppendTo, const Utf8String &u8sSrc);

class HttpClient : NO_COPY {
private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	// bAutoProxy   pwszProxy  描述
	//   false         空      不使用代理服务器
	//   true          空      使用 IE 的代理服务器
	//   false        非空     使用指定的代理服务器（pwszProxy = user:pass@addr:port|bypass）
	//   true         非空     使用指定的 PAC（pwszProxy = URL）
	explicit HttpClient(
		bool bAutoProxy = false,
		const wchar_t *pwszProxy = nullptr,
		std::size_t uProxyLen = (std::size_t)-1,
		const wchar_t *pwszUserAgent = L"MCF HTTPClient"
	);
	HttpClient(
		bool bAutoProxy,
		const Utf16String &wcsProxy,
		const wchar_t *pwszUserAgent = L"MCF HTTPClient"
	);
	HttpClient(HttpClient &&rhs) noexcept;
	HttpClient &operator=(HttpClient &&rhs) noexcept;
	~HttpClient();

public:
	bool AddCookie(
		Utf8String			u8sName,
		const Utf8String &	u8sValue,
		std::uint64_t		u64Expires,
		Utf8String			u8sPath,
		Utf8String			u8sDomain,
		bool				bSecure,
		bool				bHttpOnly
	);
	bool RemoveCookie(
		const std::pair<const char *, std::size_t> &vName,
		const std::pair<const char *, std::size_t> &vPath,
		const std::pair<const char *, std::size_t> &vDomain
	) noexcept;
	bool RemoveCookie(
		const Utf8String &u8sName,
		const Utf8String &u8sPath,
		const Utf8String &u8sDomain
	) noexcept;
	void ClearCookies() noexcept;

	Vector<Vector<unsigned char>> ExportCookies(bool bIncludeSessionOnly = false) const;
	void ImportCookies(const Vector<Vector<unsigned char>> &vecData);

	unsigned long ConnectNoThrow(
		const wchar_t *		pwszVerb,
		const wchar_t *		pwchUrl,
		std::size_t			uUrlLen = (std::size_t)-1,
		const void *		pContents = nullptr,
		std::size_t			uContentSize = 0,
		const wchar_t *		pwszContentType = L"application/x-www-form-urlencoded; charset=utf-8"
	);
	unsigned long ConnectNoThrow(
		const wchar_t *		pwszVerb,
		const Utf16String &	wcsUrl,
		const void *		pContents = nullptr,
		std::size_t			uContentSize = 0,
		const wchar_t *		pwszContentType = L"application/x-www-form-urlencoded; charset=utf-8"
	);
	void Connect(
		const wchar_t *		pwszVerb,
		const wchar_t *		pwchUrl,
		std::size_t			uUrlLen = (std::size_t)-1,
		const void *		pContents = nullptr,
		std::size_t			uContentSize = 0,
		const wchar_t *		pwszContentType = L"application/x-www-form-urlencoded; charset=utf-8"
	);
	void Connect(
		const wchar_t *		pwszVerb,
		const Utf16String &	wcsUrl,
		const void *		pContents = nullptr,
		std::size_t			uContentSize = 0,
		const wchar_t *		pwszContentType = L"application/x-www-form-urlencoded; charset=utf-8"
	);
	void Disconnect() noexcept;

/*	std::uint32_t GetStatusCode() const;
	std::size_t Read(void *pData, std::size_t uSize);*/
};

}

#endif
