// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "FileOutputStream.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

FileOutputStream::~FileOutputStream(){
}

void FileOutputStream::Put(unsigned char byData){
	Put(&byData, 1);
}

void FileOutputStream::Put(const void *pData, std::size_t uSize){
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
		const auto uBytesWritten = x_vFile.Write(x_u64Offset + uBytesTotal, pbyData + uBytesTotal, uBytesToWrite);
		if(uBytesWritten == 0){
			DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "FileOutputStream: Partial contents written"_rcs);
		}
		uBytesTotal += uBytesWritten;
	}
	x_u64Offset += uBytesTotal;
}

void FileOutputStream::Flush(bool bHard) const {
	if(bHard){
		x_vFile.HardFlush();
	}
}

}
