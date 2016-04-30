// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardErrorStream.hpp"
#include "../../MCFCRT/env/standard_streams.h"
#include "../Core/Exception.hpp"

namespace MCF {

StandardErrorStream::~StandardErrorStream(){
}

void StandardErrorStream::Put(unsigned char byData){
	Put(&byData, 1);
}
void StandardErrorStream::Put(const void *pData, std::size_t uSize){
	const auto nResult = ::_MCFCRT_WriteStandardErrorAsBinary(pData, uSize);
	if(nResult < 0){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorAsBinary() 失败。"));
	}
	const auto uWritten = static_cast<std::size_t>(nResult);
	if(uWritten < uSize){
		MCF_THROW(Exception, ERROR_BROKEN_PIPE, Rcntws::View(L"StandardErrorStream: 未能成功写入所有数据。"));
	}
}
void StandardErrorStream::Flush(bool bHard){
	(void)bHard;
}

void StandardErrorStream::PutText(wchar_t wcData){
	PutText(&wcData, 1, false);
}
void StandardErrorStream::PutText(const wchar_t *pwcData, std::size_t uSize, bool bAppendNewLine){
	const auto nResult = ::_MCFCRT_WriteStandardErrorAsText(pwcData, uSize, bAppendNewLine);
	if(nResult < 0){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorAsBinary() 失败。"));
	}
	const auto uWritten = static_cast<std::size_t>(nResult);
	if(uWritten < uSize){
		MCF_THROW(Exception, ERROR_BROKEN_PIPE, Rcntws::View(L"StandardErrorStream: 未能成功写入所有数据。"));
	}
}

}
