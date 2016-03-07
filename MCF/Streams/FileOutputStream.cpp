// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "FileOutputStream.hpp"
#include "../Core/Exception.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

FileOutputStream::~FileOutputStream(){
}

std::size_t FileOutputStream::X_WriteFromCurrentOffset(const void *pData, std::size_t uSize){
	const auto pbyData = static_cast<const unsigned char *>(pData);
	std::size_t uBytesTotal = 0;
	for(;;){
		auto uBytesToWrite = Min(uSize - uBytesTotal, UINT32_MAX);
		if(uBytesToWrite == 0){
			break;
		}
		const auto uBytesWritten = x_vFile.Write(x_u64Offset + uBytesTotal, pbyData + uBytesTotal, uBytesToWrite);
		uBytesTotal += uBytesWritten;
		if(uBytesWritten < uBytesToWrite){
			break;
		}
	}
	return uBytesTotal;
}
void FileOutputStream::X_Flush(bool bHard){
	if(bHard){
		x_vFile.HardFlush();
	}
}

void FileOutputStream::Put(unsigned char byData){
	const auto uBytesWritten = X_WriteFromCurrentOffset(&byData, 1);
	if(uBytesWritten < 1){
		DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "FileOutputStream: Partial contents written"_rcs);
	}
	x_u64Offset += 1;
}

void FileOutputStream::Put(const void *pData, std::size_t uSize){
	const auto uBytesWritten = X_WriteFromCurrentOffset(pData, uSize);
	if(uBytesWritten < uSize){
		DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "FileOutputStream: Partial contents written"_rcs);
	}
	x_u64Offset += uBytesWritten;
}

void FileOutputStream::Flush(bool bHard){
	X_Flush(bHard);
}

}
