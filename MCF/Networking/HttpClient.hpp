// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_HTTP_CLIENT_HPP_
#define MCF_HTTP_CLIENT_HPP_

#include "../Core/VVector.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/String.hpp"
#include <memory>
#include <cstddef>
#include <cstdint>

namespace MCF {

class HttpClient : NO_COPY, ABSTRACT {
public:
	// m_bAutoConfig  m_wcsProxy  描述
	// false          空          不使用代理服务器
	// true           空          使用 IE 的代理服务器
	// false          命名代理    使用指定的代理服务器（user:pass@addr:port|bypass1;bypass2;...）
	// true           PAC URL     使用指定的 PAC
	std::unique_ptr<HttpClient> Create(
		bool bAutoConfig		= false,
		WideString wcsProxy		= WideString(),
		WideString wcsUserAgent	= WideString(L"MCF HTTPClient")
	);

public:
/*	void Connect(
		const wchar_t *				pwszVerb,
		const WideStringObserver &	wsoUrl,
		const void *				pContents = nullptr,
		std::size_t					uContentSize = 0,
		const WideStringObserver &	wsoContentType = L"application/x-www-form-urlencoded; charset=utf-8"
	);
	void ConnectNoThrow(
		const wchar_t *				pwszVerb,
		const WideStringObserver &	wsoUrl,
		const void *				pContents = nullptr,
		std::size_t					uContentSize = 0,
		const WideStringObserver &	wsoContentType = L"application/x-www-form-urlencoded; charset=utf-8"
	);
	void Disconnect() noexcept;

	unsigned int GetStatusCode() const;
	std::size_t Read(void *pData, std::size_t uSize);
*/
};

}

#endif
