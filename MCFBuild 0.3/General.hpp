// Copyleft 2013, LH_Mouse. All wrongs reserved.

#ifndef __GENERAL_HPP__
#define __GENERAL_HPP__

#ifndef NDEBUG
#	define _GLIBCXX_DEBUG	1
#endif

#define WIN32_LEAN_AND_MEAN

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
// F*ck std::basic_string, f*ck copy-on-write.
#include <ext/vstring.h>
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
	typedef ::__gnu_cxx::__versa_string<char> vstring;
	typedef ::__gnu_cxx::__versa_string<wchar_t> wvstring;

	extern int Print(const wchar_t *s, std::size_t len = (std::size_t)-1, bool to_stderr = false);
	extern int VPrint(const wchar_t *fmt, bool to_stderr = false, ...);
	extern int PrintCR(const wchar_t *s, std::size_t len = (std::size_t)-1);
	extern int VPrintCR(const wchar_t *fmt, ...);

	CRITICAL_SECTION &GetPrintLock();

	template<typename... T>
	static inline int Output(const wchar_t *fmt, T... args){
		return VPrint(fmt, false, args...);
	}
	static inline int Output(const wvstring &wcs){
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
	static inline int Error(const wvstring &wcs){
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
	static inline int Status(const wvstring &wcs){
		return PrintCR(wcs.c_str(), wcs.size());
	}
	static inline int Status(const wchar_t *s){
		return PrintCR(s, -1);
	}
	static inline int Status(){
		return Status(L"");
	}

	struct Exception {
		unsigned long ulCode;
		wvstring wcsDescription;
	};

	extern vstring WcsToU8s(const wvstring &src);
	extern wvstring U8sToWcs(const vstring &src);
	extern wvstring U8sToWcs(const std::string &src);

	struct CriticalSection : public CRITICAL_SECTION {
		CriticalSection(){
			::InitializeCriticalSectionAndSpinCount(this, 0x400);
		}
		~CriticalSection(){
			::DeleteCriticalSection(this);
		}

		CriticalSection(const CriticalSection &) = delete;
		void operator=(const CriticalSection &) = delete;
	};

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

	struct WcsComparerNoCase {
		bool operator()(const wvstring &lhs, const wvstring &rhs) const {
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
		struct PRECOMPILED_HEADER {
			wvstring wcsSourceFile;
			wvstring wcsCommandLine;
		};
		struct COMPILER {
			wvstring wcsCommandLine;
			wvstring wcsDependency;
		};
		struct LINKERS {
			wvstring wcsPartial;
			wvstring wcsFull;
		};

		long long llProjectFileTimestamp;
		std::set<wvstring, WcsComparerNoCase> setIgnoredFiles;
		std::map<wvstring, PRECOMPILED_HEADER, WcsComparerNoCase> mapPreCompiledHeaders;
		std::map<wvstring, COMPILER, WcsComparerNoCase> mapCompilers;
		LINKERS Linkers;
		wvstring wcsOutputPath;
	};

	struct FOLDER_TREE {
		std::map<wvstring, FOLDER_TREE, WcsComparerNoCase> mapSubFolders;
		std::map<wvstring, long long, WcsComparerNoCase> mapFiles;
	};

	struct BUILD_JOBS {
		struct GCH {
			wvstring wcsSourceFile;
			wvstring wcsCommandLine;
			wvstring wcsStubFile;
		};

		std::map<wvstring, GCH> mapGCHs;
		std::list<wvstring> lstFilesToCompile;
		std::list<wvstring> lstFilesToLink;
	};

	extern void FixPath(wvstring &wcsSrc);
	extern wvstring GetFileExtension(const wvstring &wcsPath);
	extern wvstring MakeCommandLine(
		const wvstring &wcsBase,
		const std::map<wvstring, wvstring> &mapReplacements,
		const wchar_t *pwszPrefix
	);
	extern void TouchFolder(const wvstring &wcsPath);

	struct PROCESS_EXIT_INFO {
		DWORD dwExitCode;
		vstring strStdOut;
		vstring strStdErr;
	};

	PROCESS_EXIT_INFO Execute(const wvstring &wcsCmdLine);
}

#endif
