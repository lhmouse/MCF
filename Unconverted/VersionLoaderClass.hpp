// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_VERSION_LOADER_CLASS_HPP__
#define __MCF_VERSION_LOADER_CLASS_HPP__

#include "StdMCF.hpp"

namespace MCF {
	class VersionLoaderClass final : NO_COPY_OR_ASSIGN {
	private:
		static BOOL CALLBACK xEnumVersionResNameProc(HMODULE hModule, LPCTSTR pszType, LPTSTR pszName, LPARAM lParam);
	private:
		LPCTSTR xm_pszResourceName;
		WORD xm_arwVersionBuffer[4];
	public:
		VersionLoaderClass(HMODULE hModule);
		~VersionLoaderClass();
	public:
		const WORD &operator[](std::size_t uIndex) const;
	};
}

#endif
