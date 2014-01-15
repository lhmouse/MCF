// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_HTTP_CLASS_HPP__
#define __MCF_HTTP_CLASS_HPP__

#include "StdMCF.hpp"
#include <vector>
#include <map>

namespace MCF {
	class HTTPClass : NO_COPY_OR_ASSIGN {
	private:
		class xHTTPThreadClass;
	public:
		typedef enum {
			NONE,
			USE_IE_PROXY,
			MANUAL
		} PROXYTYPE;

		typedef struct xtagCookieData {
			WCString wcsValue;
			WCString wcsDomain;
			WCString wcsPath;
			// 将过期时间的 wYear 成员设为 0 表明这是一个仅在会话期间存在的 Cookie。
			SYSTEMTIME stExpires;
			bool bSecure;
			bool bHttpOnly;
		} COOKIEDATA;

		typedef std::map<WCString, COOKIEDATA> COOKIEMAP;

		typedef struct xtagMIMEType {
			TString strType;
			TString strSubType;
			std::map<TString, TString> mapParameters;
		} MIMETYPE;
	private:
		static const wchar_t xDAY_OF_WEEK_TABLE[][4];
		static const wchar_t xMONTH_TABLE[][4];
	public:
		static const DWORD INVALID_DWORD_VALUE = (DWORD)-1;
	private:
		static WCString xCookieToWCString(LPCWSTR pwszCookieName, const COOKIEDATA &CookieData);
		static WCString xCookieFromWCString(COOKIEDATA &CookieDataRet, LPCWSTR pwszCookieString);
		static bool xIsCookieExpired(const COOKIEDATA &CookieData);

		static WCString xSystemTimeToWCString(const SYSTEMTIME &SystemTimeSrc);
		static SYSTEMTIME xSystemTimeFromWCString(LPCWSTR pwszTimeString);

		static WCString xMIMETypeToWCString(const MIMETYPE &MIMETypeSrc);
		static MIMETYPE xMIMETypeFromWCString(LPCWSTR pszMIMETypeString);
	public:
		static UTF8String PercentEncode(const char *pszToEncode);
		static UTF8String PercentDecode(const char *pszToDecode);
	private:
		xHTTPThreadClass *xm_pHTTPThread;

		Event xm_evnHeaderAvailable;

		WCString xm_wcsUserAgent;

		BYTE xm_abyProxyIP[4];
		DWORD xm_dwProxyPort;
		WCString xm_wcsProxyUserName;
		WCString xm_wcsProxyPassword;

		int xm_nResolveTimeout;
		int xm_nConnectTimeout;
		int xm_nSendTimeout;
		int xm_nReceiveTimeout;

		// 在调用 xPerformRequest 之前先填写这些数据。
		COOKIEMAP xm_mapCookie;

		DWORD xm_dwErrorCode;
		DWORD xm_dwStatusCode;
		DWORD xm_dwBytesContentLength;
		MIMETYPE xm_ContentType;

		std::map<WCString, WCString> xm_mapHeaders;

		ReadWriteLock xm_rwlReceivedDataLock;
		std::vector<BYTE> xm_vecbyReceivedData;
	public:
		HTTPClass(bool bAsyncMode = false);
		virtual ~HTTPClass();
	protected:
		void xInternalFlushCookie();
		const BYTE *xInternalGetDataPtr() const;
		DWORD xInternalGetDataSize() const;

		virtual void xPostCreateRequest();
		virtual void xPostSendRequest();
		virtual void xPreTransferContent();
		virtual void xPostTransferContent();
		virtual void xPreShutdownRequest();
	public:
		DWORD PeekStatusCode() const;

		bool IsHeaderAvailable() const;
		void WaitForHeader() const;
		void WaitForHeaderTimeout(DWORD dwMilliSeconds) const;
		bool IsIdle() const;
		void Wait() const;
		bool WaitTimeout(DWORD dwMilliSeconds) const;

		// 以下操作需要等待 IsHeaderAvailable() 返回 true。
		const MIMETYPE &GetContentType() const;
		DWORD GetBytesTotal() const;
		DWORD GetBytesReceived() const;
		DWORD GetStatusCode() const;

		// 以下操作需要等待 IsIdle() 返回 true。
		void SetUserAgent(LPCTSTR pszNewUserAgent);
		void SetProxy(BYTE byIP1, BYTE byIP2, BYTE byIP3, BYTE byIP4, DWORD dwPort, LPCTSTR pszUserName = nullptr, LPCTSTR pszPassword = nullptr);
		void ClearProxy();
		void SetTimeouts(int nResolveTimeout = 0, int nConnectTimeout = 60000, int nSendTimeout = 30000, int nReceiveTimeout = 30000);

		DWORD GetErrorCode() const;

		TString GetHeader(LPCTSTR pszHeaderName) const;

		const BYTE *GetDataPtr() const;
		DWORD GetDataSize() const;

		void FlushCookie();
		void ClearCookie();

		TString ExportCookie(bool bIgnoreSessionOnly, bool bIgnoreHttpOnly) const;
		void ImportCookie(LPCTSTR pszExportedCookie);
		TString GetCookie(LPCTSTR pszName) const;
		void SetCookie(LPCTSTR pszName, LPCTSTR pszValue);

		const COOKIEMAP &ExportRawCookie() const;
		void ImportRawCookie(COOKIEMAP &&mapExportedCookie);
		bool GetRawCookie(COOKIEDATA &CookieDataRet, LPCWSTR pwszName) const;
		void SetRawCookie(LPCWSTR pwszName, COOKIEDATA &&CookieData);

		bool Get(LPCTSTR pszURL);
		bool Head(LPCTSTR pszURL);
		bool Post(LPCTSTR pszURL, const void *pDataToPost, std::size_t uBytesToPost, LPCTSTR pszPostMIMEType = _T("application/x-www-form-urlencoded; charset=utf-8"));
		void Shutdown();
	};
}

#endif
