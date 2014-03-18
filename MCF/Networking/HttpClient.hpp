// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_HTTP_CLIENT_HPP__
#define __MCF_HTTP_CLIENT_HPP__

#include "../Core/NoCopy.hpp"
#include "../Core/String.hpp"
#include "../../MCFCRT/cpp/ext/vvector.hpp"
#include "../../MCFCRT/cpp/ext/multi_indexed_map.hpp"
#include "PeerInfo.hpp"
#include <memory>
#include <deque>
#include <cstddef>
#include <cstdint>

namespace MCF {

class HttpClient : NO_COPY {
private:
	class xDelegate;

public:
	struct CookieItem {
		UTF16String u16sValue;
		bool bSecure;
		bool bHttpOnly;
	};

	typedef MultiIndexedMap<
		CookieItem,
		UTF16String,	// name
		UTF16String,	// path
		UTF16String,	// domain
		std::uint64_t	// expires
	> CookieMap;

	struct MimeData {
		UTF16String u16sMimeType;
		std::deque<unsigned char> deqData;
	};

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	HttpClient();
	~HttpClient();

public:
	const PeerInfo &GetProxy() const noexcept;
	void SetProxy(const PeerInfo &vProxyInfo) noexcept;
	void SetProxyAuthorization(UTF16String u16sUsername, UTF16String u16sPassword) noexcept;

	void SetAuthorization(UTF16String u16sUsername, UTF16String u16sPassword) noexcept;

	const std::map<CookieKey, CookieItem> &GetCookies() const noexcept;
	std::map<CookieKey, CookieItem> &GetCookies() noexcept;

	const MimeData &GetOutgoingData() const noexcept;
	MimeData &GetOutgoingData() noexcept;

	unsigned long ConnectNoThrow(const wchar_t *pwszVerb, const wchar_t *pwszUrl, std::size_t uUrlLen = (std::size_t)-1);
	void Connect(const wchar_t *pwszVerb, const wchar_t *pwszUrl, std::size_t uUrlLen = (std::size_t)-1);

	unsigned int GetStatusCode() const;
	std::size_t Read(void *pData, std::size_t uSize);
	void Disconnect() noexcept;
};

}

#endif
