// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardOutputStream.hpp"
#include "../../MCFCRT/env/standard_streams.h"
#include "../Core/Exception.hpp"

namespace MCF {

StandardOutputStream::~StandardOutputStream(void){
}

void StandardOutputStream::Put(unsigned char byData){
	const auto bWritten = ::_MCFCRT_WriteStandardOutputByte(byData);
	if(!bWritten){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputByte() 失败。"));
	}
}
void StandardOutputStream::Put(const void *pData, std::size_t uSize){
	const auto uWritten = ::_MCFCRT_WriteStandardOutputBinary(pData, uSize);
	if(uWritten == 0){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputBinary() 失败。"));
	}
	if(uWritten < uSize){
		MCF_THROW(Exception, ERROR_BROKEN_PIPE, Rcntws::View(L"StandardOutputStream: 未能成功写入所有数据。"));
	}
}
void StandardOutputStream::Flush(bool bHard){
	if(!::_MCFCRT_FlushStandardOutput(bHard)){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_FlushStandardOutput() 失败。"));
	}
}

void StandardOutputStream::PutChar32(char32_t c32Data){
	const auto bWritten = ::_MCFCRT_WriteStandardOutputChar32(c32Data);
	if(!bWritten){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputChar32() 失败。"));
	}
}
void StandardOutputStream::PutString(const wchar_t *pwcData, std::size_t uSize, bool bAppendNewLine){
	const auto uWritten = ::_MCFCRT_WriteStandardOutputString(pwcData, uSize, bAppendNewLine);
	if(uWritten == 0){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputString() 失败。"));
	}
	if(uWritten < uSize){
		MCF_THROW(Exception, ERROR_BROKEN_PIPE, Rcntws::View(L"StandardOutputStream: 未能成功写入所有数据。"));
	}
}

bool StandardOutputStream::IsBuffered() const noexcept {
	return ::_MCFCRT_IsStandardOutputBuffered();
}
bool StandardOutputStream::SetBuffered(bool bBuffered) noexcept {
	return ::_MCFCRT_SetStandardOutputBuffered(bBuffered);
}

}
