// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "ResourceLoaderClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
ResourceLoaderClass::ResourceLoaderClass(HMODULE hModule, int nResourceID, LPCTSTR pszResourceType){
	const HRSRC hResource = VERIFY(FindResource(hModule, MAKEINTRESOURCE(nResourceID), pszResourceType));

	xm_pData = VERIFY(LockResource(LoadResource(hModule, hResource)));
	xm_dwSize =  SizeofResource(hModule, hResource);
}
ResourceLoaderClass::~ResourceLoaderClass(){
}

// 其他非静态成员函数。
LPCVOID ResourceLoaderClass::GetDataPtr() const {
	return xm_pData;
}
DWORD ResourceLoaderClass::GetDataSize() const {
	return xm_dwSize;
}
