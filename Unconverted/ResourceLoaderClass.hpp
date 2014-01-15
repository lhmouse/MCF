// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_RESOURCE_LOADER_CLASS_HPP__
#define __MCF_RESOURCE_LOADER_CLASS_HPP__

#include "StdMCF.hpp"

namespace MCF {
	class ResourceLoaderClass final : NO_COPY_OR_ASSIGN {
	private:
		LPCVOID xm_pData;
		DWORD xm_dwSize;
	public:
		ResourceLoaderClass(HMODULE hModule, int nResourceID, LPCTSTR pszResourceType);
		~ResourceLoaderClass();
	public:
		LPCVOID GetDataPtr() const;
		DWORD GetDataSize() const;
	};
}

#endif
