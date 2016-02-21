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
	std::size_t uBytesWritten = 0;
	while(uBytesWritten < uSize){
		const auto uBytesWrittenThisTime = x_vFile.Write(x_u64Offset + uBytesWritten, static_cast<const unsigned char *>(pData) + uBytesWritten, uSize - uBytesWritten);
		if(uBytesWrittenThisTime == 0){
			DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "FileOutputStream: Partial contents written"_rcs);
		}
		uBytesWritten += uBytesWrittenThisTime;
	}
	x_u64Offset += uBytesWritten;
}

void FileOutputStream::Flush() const {
	x_vFile.Flush();
}

}
