// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "StandardErrorStream.hpp"
#include "../../MCFCRT/env/standard_streams.h"
#include "../Core/Exception.hpp"

namespace MCF {

StandardErrorStream::~StandardErrorStream(){
}

void StandardErrorStream::Put(unsigned char byData){
	const auto bSucceeded = ::_MCFCRT_WriteStandardErrorByte(byData);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorByte() 失败。"));
	}
}
void StandardErrorStream::Put(const void *pData, std::size_t uSize){
	const auto bSucceeded = ::_MCFCRT_WriteStandardErrorBinary(pData, uSize);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorBinary() 失败。"));
	}
}
void StandardErrorStream::Flush(bool bHard){
	(void)bHard;
}

void StandardErrorStream::PutChar32(char32_t c32Data){
	const auto bSucceeded = ::_MCFCRT_WriteStandardErrorChar32(c32Data);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorChar32() 失败。"));
	}
}
void StandardErrorStream::PutString(const wchar_t *pwcData, std::size_t uSize, bool bAppendNewLine){
	const auto bSucceeded = ::_MCFCRT_WriteStandardErrorString(pwcData, uSize, bAppendNewLine);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorString() 失败。"));
	}
}

}
