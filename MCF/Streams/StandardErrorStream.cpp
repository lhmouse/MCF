// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardErrorStream.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

namespace {
	HANDLE GetStdErrorHandle(){
		const auto hPipe = ::GetStdHandle(STD_ERROR_HANDLE);
		if(hPipe == INVALID_HANDLE_VALUE){
			const auto dwLastError = ::GetLastError();
			DEBUG_THROW(SystemException, dwLastError, "GetStdHandle"_rcs);
		}
		return hPipe;
	}
}

StandardErrorStream::StandardErrorStream()
	: x_hPipe(GetStdErrorHandle())
{
}
StandardErrorStream::~StandardErrorStream(){
}

void StandardErrorStream::Put(unsigned char byData){
	Put(&byData, 1);
}

void StandardErrorStream::Put(const void *pData, std::size_t uSize){
	if(!x_hPipe){
		return;
	}

	const auto pbyData = static_cast<const unsigned char *>(pData);
	std::size_t uBytesTotal = 0;
	for(;;){
		auto uBytesToWrite = uSize - uBytesTotal;
		if(uBytesToWrite == 0){
			break;
		}
		if(uBytesToWrite > UINT32_MAX){
			uBytesToWrite = UINT32_MAX;
		}
		DWORD dwBytesWritten;
		if(!::WriteFile(x_hPipe, pbyData + uBytesTotal, uBytesToWrite, &dwBytesWritten, nullptr)){
			const auto dwLastError = ::GetLastError();
			DEBUG_THROW(SystemException, dwLastError, "WriteFile"_rcs);
		}
		uBytesTotal += dwBytesWritten;
	}
}

void StandardErrorStream::Flush(bool bHard) const {
	if(!x_hPipe){
		return;
	}

	if(bHard){
		if(!::FlushFileBuffers(x_hPipe)){
			const auto dwLastError = ::GetLastError();
			if((dwLastError != ERROR_INVALID_FUNCTION) && (dwLastError != ERROR_INVALID_HANDLE)){
				DEBUG_THROW(SystemException, dwLastError, "FlushFileBuffers"_rcs);
			}
		}
	}
}

}
