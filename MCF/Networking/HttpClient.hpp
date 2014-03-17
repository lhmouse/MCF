// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_HTTP_CLIENT_HPP__
#define __MCF_HTTP_CLIENT_HPP__

#include "../Core/NoCopy.hpp"
#include "../Core/String.hpp"
#include "../../MCFCRT/cpp/ext/vvector.hpp"
#include "PeerInfo.hpp"
#include <memory>
#include <map>
#include <vector>
#include <cstddef>
#include <cstdint>
/*
namespace MCF {

class HttpClient : NO_COPY {
private:
	class xDelegate;

public:
	struct CookieKey {
		UTF16String u16sName;
		VVector<UTF16String, 8> u16sPath;
		VVector<UTF16String, 4> vecDomain;

		bool operator<(const CookieKey &rhs) const noexcept {
			const std::size_t uPathSize = u16sPath.GetSize();
			if(uPathSize != rhs.u16sPath.GetSize()){
				return uPathSize < rhs.u16sPath.GetSize();
			}
			for(std::size_t i = 0; i < uPathSize; ++i){
				const int nResult = u16sPath[i].Compare(rhs.u16sPath[i]);
				if(nResult != 0){
					return nResult < 0;
				}
			}
			return u16sName < rhs.u16sName;
		}
	};

	struct CookieItem {
		UTF16String u16sValue;
		std::uint64_t u64Expires;
		bool bSecure;
		bool bHttpOnly;
	};

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

	const MimeData &GetIncomingData() const noexcept;
	MimeData &GetIncomingData() noexcept;

	unsigned long ConnectNoThrow(const wchar_t *pwszVerb, const wchar_t *pwszUrl, std::size_t uUrlLen = (std::size_t)-1);
	void Connect(const wchar_t *pwszVerb, const wchar_t *pwszUrl, std::size_t uUrlLen = (std::size_t)-1);

	unsigned int GetStatusCode() const;
	const MimeData &GetIncomingData() const noexcept;
	void Disconnect() noexcept;
};

}
*/
#endif
