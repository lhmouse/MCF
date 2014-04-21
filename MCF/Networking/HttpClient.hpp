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

class HttpClient : NO_COPY, ABSTRACT {
public:
	// bAutoProxy   u8sProxy   描述
	//   false         空      不使用代理服务器
	//   true          空      使用 IE 的代理服务器
	//   false        非空     使用指定的代理服务器（u8sProxy = user:pass@addr:port|bypass）
	//   true         非空     使用指定的 PAC（u8sProxy = URL）
	std::unique_ptr<HttpClient> Create(
		bool bUseAutoProxy = false,
		WideString wcsProxy = WideString(),
		WideString wcsUserAgent = WideString(L"MCF HTTPClient")
	);

public:
/*	void Connect(
		const WideStringObserver &	wsoVerb,
		const WideStringObserver &	wsoUrl,
		const void *				pContents = nullptr,
		std::size_t					uContentSize = 0,
		const WideStringObserver &	wsoContentType = L"application/x-www-form-urlencoded; charset=utf-8"
	);
	unsigned long ConnectNoThrow(
		const WideStringObserver &	wsoVerb,
		const WideStringObserver &	wsoUrl,
		const void *				pContents = nullptr,
		std::size_t					uContentSize = 0,
		const WideStringObserver &	wsoContentType = L"application/x-www-form-urlencoded; charset=utf-8"
	);
	void Disconnect() noexcept;

	std::uint32_t GetStatusCode() const;
	std::size_t Read(void *pData, std::size_t uSize);*/
};

}

#endif
