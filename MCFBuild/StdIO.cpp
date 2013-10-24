// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include <cstdarg>
#include <memory>
#include <vector>
#include <algorithm>

namespace {
	class PrintLock {
	public:
		CRITICAL_SECTION m_cs;
	public:
		PrintLock(){
			::InitializeCriticalSectionAndSpinCount(&m_cs, 0x400);
		}
		~PrintLock(){
			::DeleteCriticalSection(&m_cs);
		}
	} g_PrintLock;
}

namespace MCFBuild {
	int Print(const wchar_t *s, std::size_t len, bool to_stderr){
		int nRet = -1;
		const HANDLE hOut = ::GetStdHandle(to_stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
		if(hOut != NULL){
			if(len == (std::size_t)-1){
				len = std::wcslen(s);
			}
			std::wstring wcsTemp;
			wcsTemp.reserve(len + 1);
			wcsTemp.append(s, len);
			wcsTemp.push_back(L'\n');

			DWORD dwBytesWritten;
			DWORD dwMode;
			if(::GetConsoleMode(hOut, &dwMode) != FALSE){
				LOCK_THROUGH(g_PrintLock.m_cs);

				nRet = ::WriteConsoleW(hOut, wcsTemp.c_str(), wcsTemp.size(), &dwBytesWritten, nullptr);
			} else {
				const std::string u8sConvtered = WcsToU8s(wcsTemp);
				if(!u8sConvtered.empty()){
					LOCK_THROUGH(g_PrintLock.m_cs);

					nRet = ::WriteFile(hOut, u8sConvtered.c_str(), u8sConvtered.size(), &dwBytesWritten, nullptr);
				}
			}
		}
		return nRet;
	}
	int VPrint(const wchar_t *fmt, bool to_stderr, ...){
		std::va_list ap;

		wchar_t achBuffer[256];

		va_start(ap, to_stderr);
		const int nCount = std::vswprintf(achBuffer, COUNTOF(achBuffer), fmt, ap);
		va_end(ap);

		if(nCount >= 0){
			return Print(achBuffer, (std::size_t)nCount, to_stderr);
		}

		std::unique_ptr<wchar_t> pwchBuffer(new wchar_t[nCount + 1]);

		va_start(ap, to_stderr);
		const int nNewCount = std::vswprintf(pwchBuffer.get(), nCount + 1, fmt, ap);
		va_end(ap);

		if(nNewCount < 0){
			return nNewCount;
		}
		return Print(pwchBuffer.get(), (std::size_t)nNewCount, to_stderr);
	}
	int PrintCR(const wchar_t *s, std::size_t len){
		int nRet = -1;
		const HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if(hOut != NULL){
			DWORD dwMode;
			if(::GetConsoleMode(hOut, &dwMode) != FALSE){
				if(len == (std::size_t)-1){
					len = std::wcslen(s);
				}
				std::wstring wcsTemp;
				wcsTemp.reserve(len + 1);
				wcsTemp.append(s, len);
				wcsTemp.push_back(L'\r');

				LOCK_THROUGH(g_PrintLock.m_cs);

				DWORD dwBytesWritten;
				nRet = ::WriteConsoleW(hOut, wcsTemp.c_str(), wcsTemp.size(), &dwBytesWritten, nullptr);
			}
		}
		return nRet;
	}
	int VPrintCR(const wchar_t *fmt, ...){
		std::va_list ap;

		wchar_t achBuffer[256];

		va_start(ap, fmt);
		const int nCount = std::vswprintf(achBuffer, COUNTOF(achBuffer), fmt, ap);
		va_end(ap);

		if(nCount >= 0){
			return PrintCR(achBuffer, (std::size_t)nCount);
		}

		std::unique_ptr<wchar_t> pwchBuffer(new wchar_t[nCount + 1]);

		va_start(ap, fmt);
		const int nNewCount = std::vswprintf(pwchBuffer.get(), nCount + 1, fmt, ap);
		va_end(ap);

		if(nNewCount < 0){
			return nNewCount;
		}
		return PrintCR(pwchBuffer.get(), (std::size_t)nNewCount);
	}

	CRITICAL_SECTION &GetPrintLock(){
		return g_PrintLock.m_cs;
	}
}
