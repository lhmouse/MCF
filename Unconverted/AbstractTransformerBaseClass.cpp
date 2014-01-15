// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "AbstractTransformerBaseClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
AbstractTransformerBaseClass::AbstractTransformerBaseClass(){
	xm_bStreamEnded = true;
}
AbstractTransformerBaseClass::~AbstractTransformerBaseClass(){
}

// 其他非静态成员函数。
void AbstractTransformerBaseClass::xTransform(void *pOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	if(xm_bStreamEnded){
		xInitialize(pOutput);

		xm_bStreamEnded = false;
	}

	xUpdate(pOutput, pbyData, uSize);

	if(bIsEndOfStream){
		xFinalize(pOutput);

		xm_bStreamEnded = true;
	}
}
