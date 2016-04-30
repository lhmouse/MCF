// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardOutputStream.hpp"
#include "../../MCFCRT/env/standard_streams.h"
#include "../Core/Exception.hpp"

namespace MCF {

StandardOutputStream::~StandardOutputStream(){
}

void StandardOutputStream::Put(unsigned char byData){
	Put(&byData, 1);
}
void StandardOutputStream::Put(const void *pData, std::size_t uSize){
	const auto nResult = ::_MCFCRT_WriteStandardOutputAsBinary(pData, uSize);
	if(nResult < 0){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputAsBinary() 失败。"));
	}
	const auto uWritten = static_cast<std::size_t>(nResult);
	if(uWritten < uSize){
		MCF_THROW(Exception, ERROR_BROKEN_PIPE, Rcntws::View(L"StandardOutputStream: 未能成功写入所有数据。"));
	}
}
void StandardOutputStream::Flush(bool bHard){
	if(!::_MCFCRT_FlushStandardOutput(bHard)){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_FlushStandardOutput() 失败。"));
	}
}

void StandardOutputStream::PutText(wchar_t wcData){
	PutText(&wcData, 1, false);
}
void StandardOutputStream::PutText(const wchar_t *pwcData, std::size_t uSize, bool bAppendNewLine){
	const auto nResult = ::_MCFCRT_WriteStandardOutputAsText(pwcData, uSize, bAppendNewLine);
	if(nResult < 0){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardOutputStream: _MCFCRT_WriteStandardOutputAsBinary() 失败。"));
	}
	const auto uWritten = static_cast<std::size_t>(nResult);
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
