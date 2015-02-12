// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2015, LH_Mouse. All wrongs reserved.

#include "Precompiled.hpp"
#include "Localization.hpp"
using namespace MCFBuild;

namespace {

struct TableElement {
	const char achKey[64 - sizeof(MCF::WideStringObserver)];
	MCF::WideStringObserver wsoValue;
};

// 按字母顺序排列。
const TableElement TABLE[] = {
#define DECL(key_, val_)				{ #key_, L ## val_ ##_wso },

DECL(MCFBUILD,							"MCF 构建实用工具")
DECL(VERSION,							"版本")

};

}

MCF::WideString &Localization::Get(MCF::WideString &wcsAppendsTo, const char *pszKey){
	auto pLower = std::begin(TABLE), pUpper = std::end(TABLE);
	for(;;){
		const auto pMiddle = pLower + (pUpper - pLower) / 2;
		const auto nResult = std::strcmp(pszKey, pMiddle->achKey);
		if(nResult == 0){
			wcsAppendsTo += pMiddle->wsoValue;
			break;
		}
		if(nResult < 0){
			pUpper = pMiddle;
		} else {
			pLower = pMiddle + 1;
		}
		if(pLower == pUpper){
			break;
		}
	}
	return wcsAppendsTo;
}
