// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_HTTP_CLIENT_HPP__
#define __MCF_HTTP_CLIENT_HPP__

#include "../Core/NoCopy.hpp"
#include "../Core/String.hpp"
#include "../../MCFCRT/cpp/ext/multi_indexed_map.hpp"
#include "../../MCFCRT/cpp/ext/vvector.hpp"
#include "PeerInfo.hpp"
#include <memory>
#include <cstddef>
#include <cstdint>

namespace MCF {

class HttpClient : NO_COPY {
public:
	enum : std::uint64_t {
		UNKNOWN_CONTENT_LENGTH = (std::uint64_t)-1
	};

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
	explicit HttpClient(bool bAutoProxy = false, const wchar_t *pwszProxy = nullptr, std::size_t uProxyLen = (std::size_t)-1);
	HttpClient(bool bAutoProxy, const Utf16String &wcsProxy);
	HttpClient(HttpClient &&rhs) noexcept;
	HttpClient &operator=(HttpClient &&rhs) noexcept;
	~HttpClient();

public:
	unsigned long ConnectNoThrow(const wchar_t *pwszVerb, const wchar_t *pwszUrl, std::size_t uUrlLen = (std::size_t)-1);
	unsigned long ConnectNoThrow(const wchar_t *pwszVerb, const Utf16String &wcsUrl);
	void Connect(const wchar_t *pwszVerb, const wchar_t *pwszUrl, std::size_t uUrlLen = (std::size_t)-1);
	void Connect(const wchar_t *pwszVerb, const Utf16String &wcsUrl);
	void Disconnect() noexcept;

/*	std::uint32_t GetStatusCode() const;
	std::size_t Read(void *pData, std::size_t uSize);*/
};

}

#endif
