/*// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "HttpClient.hpp"
#include "../../MCFCRT/c/ext/assert.h"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include <winhttp.h>
using namespace MCF;

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

	typedef UniqueHandle<WinHttpCloser> xWinHttpHandle;

private:
	PeerInfo xm_vProxy;
	UTF16String xm_u16sProxyUsername;
	UTF16String xm_u16sProxyPassword;

	UTF16String xm_u16sUsername;
	UTF16String xm_u16sPassword;

	std::map<CookieKey, CookieItem> xm_mapCookies;
	std::map<UTF16String, std::vector<unsigned char>> xm_mapOutgoingData;

	xWinHttpHandle xm_hSession;
	xWinHttpHandle xm_hConnect;
	xWinHttpHandle xm_hRequest;

	unsigned int xm_uStatusCode;
	std::map<UTF16String, std::vector<unsigned char>> xm_mapIncomingData;

public:
	xDelegate() noexcept
		: xm_vProxy(true, 0)
	{
	}

public:
	const PeerInfo &GetProxy() const noexcept {
		return xm_vProxy;
	}
	void SetProxy(const PeerInfo &vProxyInfo) noexcept {
		xm_vProxy = vProxyInfo;
	}
	void SetProxyAuthorization(UTF16String &&u16sUsername, UTF16String &&u16sPassword) noexcept {
		xm_u16sProxyUsername = std::move(u16sUsername);
		xm_u16sProxyPassword = std::move(u16sPassword);
	}

	void SetAuthorization(UTF16String &&u16sUsername, UTF16String &&u16sPassword) noexcept {
		xm_u16sUsername = std::move(u16sUsername);
		xm_u16sPassword = std::move(u16sPassword);
	}

	const std::map<CookieKey, CookieItem> &GetCookies() const noexcept {
		return xm_mapCookies;
	}
	std::map<CookieKey, CookieItem> &GetCookies() noexcept {
		return xm_mapCookies;
	}

	const MimeData &GetOutgoingData() const noexcept {
	}
	MimeData &GetOutgoingData() noexcept {
	}

	unsigned long Connect(const wchar_t *pwszVerb, const wchar_t *pwszUrl, std::size_t uUrlLen){
	}

	unsigned int GetStatusCode() const {
		return xm_uStatusCode;
	}
	std::size_t Read(void *pData, std::size_t uSize){
	}
	void Disconnect() noexcept {
		xm_hRequest.Reset();
		xm_hConnect.Reset();
		xm_hSession.Reset();
	}
};

// 构造函数和析构函数。
HttpClient::HttpClient()
	: xm_pDelegate(new xDelegate)
{
}
HttpClient::~HttpClient(){
}

// 其他非静态成员函数。
const PeerInfo &HttpClient::GetProxy() const noexcept {
	return xm_pDelegate->GetProxy();
}
void HttpClient::SetProxy(const PeerInfo &vProxyInfo) noexcept {
	xm_pDelegate->SetProxy(vProxyInfo);
}
void HttpClient::SetProxyAuthorization(UTF16String u16sUsername, UTF16String u16sPassword) noexcept {
	xm_pDelegate->SetProxyAuthorization(std::move(u16sUsername), std::move(u16sPassword));
}

void HttpClient::SetAuthorization(UTF16String u16sUsername, UTF16String u16sPassword) noexcept {
	xm_pDelegate->SetAuthorization(std::move(u16sUsername), std::move(u16sPassword));
}

const std::map<HttpClient::CookieKey, HttpClient::CookieItem> &HttpClient::GetCookies() const noexcept {
	return xm_pDelegate->GetCookies();
}
std::map<HttpClient::CookieKey, HttpClient::CookieItem> &HttpClient::GetCookies() noexcept {
	return xm_pDelegate->GetCookies();
}

const HttpClient::MimeData &HttpClient::GetOutgoingData() const noexcept {
	return xm_pDelegate->GetOutgoingData();
}
HttpClient::MimeData &HttpClient::GetOutgoingData() noexcept {
	return xm_pDelegate->GetOutgoingData();
}

unsigned long HttpClient::ConnectNoThrow(const wchar_t *pwszVerb, const wchar_t *pwszUrl, std::size_t uUrlLen){
	return xm_pDelegate->Connect(pwszVerb, pwszUrl, uUrlLen);
}
void HttpClient::Connect(const wchar_t *pwszVerb, const wchar_t *pwszUrl, std::size_t uUrlLen){
	const auto ulErrorCode = ConnectNoThrow(pwszVerb, pwszUrl, uUrlLen);
	if(ulErrorCode != ERROR_SUCCESS){
		MCF_THROW(ulErrorCode, L"连接服务器失败。");
	}
}

unsigned int HttpClient::GetStatusCode() const {
	return xm_pDelegate->GetStatusCode();
}
std::size_t HttpClient::Read(void *pData, std::size_t uSize){
	return xm_pDelegate->Read(pData, uSize);
}
void HttpClient::Disconnect() noexcept {
	xm_pDelegate->Disconnect();
}
*/