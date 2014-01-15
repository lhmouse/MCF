// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_HTTP_THREAD_CLASS_HPP__
#define __MCF_X_HTTP_THREAD_CLASS_HPP__

#include "StdMCF.hpp"
#include "HTTPClass.hpp"
#include "AbstractThreadClass.hpp"
#include <vector>

namespace MCF {
	class HTTPClass::xHTTPThreadClass final : public AbstractThreadClass {
	private:
		static DWORD xSelectAuthScheme(DWORD dwSupported, DWORD dwFirst);
	private:
		HTTPClass *const xm_pHTTP;
		const bool xm_bAsyncMode;

		// Cracked URL
		int xm_nScheme;
		WCString xm_wcsHostName;
		WORD xm_wPort;
		WCString xm_wcsUserName;
		WCString xm_wcsPassword;
		WCString xm_wcsPath;
		WCString xm_wcsExtraInfo;

		WCString xm_wcsVerb;
		std::vector<BYTE> xm_vecbyPostData;
		WCString xm_wcsPostMIMEType;

		volatile bool xm_bCancelled;
	public:
		xHTTPThreadClass(HTTPClass *pHTTP, bool bAsyncMode);
		~xHTTPThreadClass();
	private:
		using AbstractThreadClass::Create;
		using AbstractThreadClass::Release;
	protected:
		virtual int xThreadProc();
	public:
		bool CreateRequest(LPCTSTR pszURL, LPCWSTR pwszVerb, const void *pDataToPost, std::size_t uBytesToPost, LPCTSTR pszPostMIMEType);
		void Cancel();
	};
}

#endif
