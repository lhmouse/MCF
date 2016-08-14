// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "StandardOutputStream.hpp"
#include <MCFCRT/env/standard_streams.h>
#include "../Core/Exception.hpp"

namespace MCF {

StandardOutputStream::~StandardOutputStream(){
}

void StandardOutputStream::Put(unsigned char byData){
	const auto bSucceeded = ::_MCFCRT_WriteStandardOutputByte(byData);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputByte() 失败。"));
	}
}
void StandardOutputStream::Put(const void *pData, std::size_t uSize){
	const auto bSucceeded = ::_MCFCRT_WriteStandardOutputBinary(pData, uSize);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputBinary() 失败。"));
	}
}
void StandardOutputStream::Flush(bool bHard){
	if(!::_MCFCRT_FlushStandardOutput(bHard)){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_FlushStandardOutput() 失败。"));
	}
}

void StandardOutputStream::PutChar32(char32_t c32Data){
	const auto bSucceeded = ::_MCFCRT_WriteStandardOutputChar32(c32Data);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputChar32() 失败。"));
	}
}
void StandardOutputStream::PutText(const wchar_t *pwcData, std::size_t uSize, bool bAppendNewLine){
	const auto bSucceeded = ::_MCFCRT_WriteStandardOutputText(pwcData, uSize, bAppendNewLine);
	if(!bSucceeded){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputText() 失败。"));
	}
}

bool StandardOutputStream::IsBuffered() const noexcept {
	return ::_MCFCRT_IsStandardOutputBuffered();
}
void StandardOutputStream::SetBuffered(bool bBuffered) noexcept {
	::_MCFCRT_SetStandardOutputBuffered(bBuffered);
}

}
