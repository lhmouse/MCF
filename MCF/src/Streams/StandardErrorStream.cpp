// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "StandardErrorStream.hpp"
#include <MCFCRT/env/standard_streams.h>
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
	if(!::_MCFCRT_FlushStandardError(bHard)){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: _MCFCRT_FlushStandardError() 失败。"));
	}
}

void StandardErrorStream::PutChar32(char32_t c32Data){
	const auto bSucceeded = ::_MCFCRT_WriteStandardErrorChar32(c32Data);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorChar32() 失败。"));
	}
}
void StandardErrorStream::PutText(const wchar_t *pwcData, std::size_t uSize, bool bAppendNewLine){
	const auto bSucceeded = ::_MCFCRT_WriteStandardErrorText(pwcData, uSize, bAppendNewLine);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorText() 失败。"));
	}
}

}
