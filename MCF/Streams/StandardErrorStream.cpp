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
	const auto uWritten = ::_MCFCRT_WriteStandardErrorAsBinary(pData, uSize);
	if(uWritten == 0){
		const auto dwErrorCode = ::GetLastError();
		MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorAsBinary() 失败。"));
	}
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
	const auto uWritten = ::_MCFCRT_WriteStandardErrorAsText(pwcData, uSize, bAppendNewLine);
	if(uWritten == 0){
		const auto dwErrorCode = ::GetLastError();
		MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardErrorStream: _MCFCRT_WriteStandardErrorAsText() 失败。"));
	}
	if(uWritten < uSize){
		MCF_THROW(Exception, ERROR_BROKEN_PIPE, Rcntws::View(L"StandardErrorStream: 未能成功写入所有数据。"));
	}
}

}
