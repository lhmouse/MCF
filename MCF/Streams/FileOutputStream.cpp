// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "FileOutputStream.hpp"
#include "../Core/Exception.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

namespace {
	std::size_t WriteAux(File &vFile, std::uint64_t u64Offset, const void *pData, std::size_t uSize){
		const auto pbyData = static_cast<const unsigned char *>(pData);
		std::size_t uBytesTotal = 0;
		for(;;){
			auto uBytesToWrite = Min(uSize - uBytesTotal, UINT32_MAX);
			if(uBytesToWrite == 0){
				break;
			}
			const auto uBytesWritten = vFile.Write(u64Offset + uBytesTotal, pbyData + uBytesTotal, uBytesToWrite);
			if(uBytesWritten == 0){
				break;
			}
			uBytesTotal += uBytesWritten;
		}
		return uBytesTotal;
	}
	void FlushAux(const File &vFile, bool bHard){
		if(bHard){
			vFile.HardFlush();
		}
	}
}

FileOutputStream::~FileOutputStream(){
}

void FileOutputStream::Put(unsigned char byData){
	const auto uBytesWritten = WriteAux(x_vFile, x_u64Offset, &byData, 1);
	if(uBytesWritten < 1){
		DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "FileOutputStream: Partial contents written"_rcs);
	}
	x_u64Offset += 1;
}

void FileOutputStream::Put(const void *pData, std::size_t uSize){
	const auto uBytesWritten = WriteAux(x_vFile, x_u64Offset, pData, uSize);
	if(uBytesWritten < uSize){
		DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "FileOutputStream: Partial contents written"_rcs);
	}
	x_u64Offset += uBytesWritten;
}

void FileOutputStream::Flush(bool bHard) const {
	FlushAux(x_vFile, bHard);
}

}
