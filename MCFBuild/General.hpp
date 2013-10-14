// Copyleft 2013, LH_Mouse. All wrongs reserved.

#ifndef __GENERAL_HPP__
#define __GENERAL_HPP__

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cwchar>
#include <cwctype>
#include <utility>
#include <string>
#include <set>
#include <map>
#include <list>
#include <type_traits>
#include <windows.h>
#include "countof.hpp"

namespace MCFBuild {
	extern DWORD g_dwMainThreadID;

	extern int Print(const wchar_t *s, std::size_t len = (std::size_t)-1, bool to_stderr = false);
	extern int VPrint(const wchar_t *fmt, bool to_stderr = false, ...);
	extern int PrintCR(const wchar_t *s, std::size_t len = (std::size_t)-1);
	extern int VPrintCR(const wchar_t *fmt, ...);

	CRITICAL_SECTION &GetPrintLock();

	struct Exception {
		unsigned long ulCode;
		std::wstring wcsDescription;

		Exception(long code, std::wstring &&desc) : ulCode((unsigned long)code), wcsDescription(std::move(desc)) { }
		Exception(unsigned long code, std::wstring &&desc) : ulCode(code), wcsDescription(std::move(desc)) { }
	};

	template<typename... T>
	static inline int Output(const wchar_t *fmt, T... args){
		return VPrint(fmt, false, args...);
	}
	static inline int Output(const std::wstring &wcs){
		return Print(wcs.c_str(), wcs.size(), false);
	}
	static inline int Output(const wchar_t *s){
		return Print(s, -1, false);
	}
	static inline int Output(){
		return Output(L"");
	}

	template<typename... T>
	static inline int Error(const wchar_t *fmt, T... args){
		return VPrint(fmt, true, args...);
	}
	static inline int Error(const std::wstring &wcs){
		return Print(wcs.c_str(), wcs.size(), true);
	}
	static inline int Error(const wchar_t *s){
		return Print(s, -1, true);
	}
	static inline int Error(){
		return Error(L"");
	}

	template<typename... T>
	static inline int Status(const wchar_t *fmt, T... args){
		return VPrintCR(fmt, args...);
	}
	static inline int Status(const std::wstring &wcs){
		return PrintCR(wcs.c_str(), wcs.size());
	}
	static inline int Status(const wchar_t *s){
		return PrintCR(s, -1);
	}
	static inline int Status(){
		return Status(L"");
	}

	static inline std::string WcsToU8s(const std::wstring &src){
		std::string ret;
		const int nConvLen = ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.size(), nullptr, 0, nullptr, nullptr);
		if(nConvLen > 0){
			ret.resize(nConvLen);
			::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.size(), &ret[0], nConvLen, nullptr, nullptr);
		}
		return std::move(ret);
	}
	static inline std::wstring U8sToWcs(const std::string &src){
		std::wstring ret;
		const int nConvLen = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), nullptr, 0);
		if(nConvLen > 0){
			ret.resize(nConvLen);
			::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), &ret[0], nConvLen);
		}
		return std::move(ret);
	}

	class CriticalSectionHelper {
	private:
		PCRITICAL_SECTION m_pCriticalSection;
	public:
		CriticalSectionHelper(PCRITICAL_SECTION pCriticalSection){
			m_pCriticalSection = pCriticalSection;

			::EnterCriticalSection(m_pCriticalSection);
		}
		CriticalSectionHelper(const CriticalSectionHelper &rhs){
			m_pCriticalSection = rhs.m_pCriticalSection;

			::EnterCriticalSection(m_pCriticalSection);
		}
		CriticalSectionHelper(CriticalSectionHelper &&rhs){
			m_pCriticalSection = rhs.m_pCriticalSection;
			rhs.m_pCriticalSection = nullptr;
		}
		CriticalSectionHelper &operator=(const CriticalSectionHelper &rhs){
			this->~CriticalSectionHelper();
			return *new(this) CriticalSectionHelper(rhs);
		}
		CriticalSectionHelper &operator=(CriticalSectionHelper &&rhs){
			this->~CriticalSectionHelper();
			return *new(this) CriticalSectionHelper(std::move(rhs));
		}
		~CriticalSectionHelper(){
			if(m_pCriticalSection != nullptr){
				::LeaveCriticalSection(m_pCriticalSection);
			}
		}
	};

#define	LOCK_THROUGH(cs)	const ::MCFBuild::CriticalSectionHelper __LOCK__(&cs)

	static inline void FixPath(std::wstring &wcsSrc){
		if(!wcsSrc.empty()){
			const DWORD dwSize = ::GetFullPathNameW(wcsSrc.c_str(), 0, nullptr, nullptr);
			std::wstring wcsTemp(dwSize, 0);
			wcsTemp.resize(::GetFullPathNameW(wcsSrc.c_str(), wcsTemp.size(), &wcsTemp[0], nullptr));
			wcsTemp.swap(wcsSrc);
		}
	};
	static inline std::wstring GetFileExtension(const std::wstring &wcsPath){
		for(std::size_t i = wcsPath.size(); i != 0; --i){
			switch(wcsPath[i - 1]){
			case L'.':
				return wcsPath.substr(i);
			case L'\\':
			case L' ':
				break;
			}
		}
		return std::wstring();
	}

	struct WcsComparerNoCase {
		bool operator()(const std::wstring &lhs, const std::wstring &rhs) const {
			auto iter1 = lhs.cbegin();
			auto iter2 = rhs.cbegin();
			for(;;){
				const auto ch1 = (std::make_unsigned<wchar_t>::type)std::towupper(*iter1);
				++iter1;
				const auto ch2 = (std::make_unsigned<wchar_t>::type)std::towupper(*iter2);
				++iter2;
				if(ch1 != ch2){
					return ch1 < ch2;
				}
				if(ch1 == 0){
					return false;
				}
			}
		}
	};

	struct PROJECT {
		struct COMPILER {
			std::wstring wcsCommandLine;
			std::wstring wcsDependency;
		};
		struct LINKERS {
			std::wstring wcsPartial;
			std::wstring wcsFull;
		};

		long long llProjectFileTimestamp;
		std::set<std::wstring, WcsComparerNoCase> setIgnored;
		std::map<std::wstring, COMPILER, WcsComparerNoCase> mapCompilers;
		LINKERS Linkers;
		std::wstring wcsOutputPath;
	};

	struct FOLDER_TREE {
		std::map<std::wstring, FOLDER_TREE, WcsComparerNoCase> mapSubFolders;
		std::map<std::wstring, long long, WcsComparerNoCase> mapFiles;
	};

	struct BUILD_JOBS {
		std::list<std::wstring> lstFilesToCompile;
		std::list<std::wstring> lstFilesToLink;
	};
}

#endif
