// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "FileOutputStream.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

FileOutputStream::~FileOutputStream(){
}

void FileOutputStream::Put(unsigned char byData){
	FileOutputStream::Put(&byData, 1);
}
void FileOutputStream::Put(const void *pData, std::size_t uSize){
	std::size_t uBytesTotal = 0;
	for(;;){
		const auto uBytesToWrite = uSize - uBytesTotal;
		if(uBytesToWrite == 0){
			break;
		}
		const auto uBytesWritten = x_vFile.Write(x_u64Offset + uBytesTotal, static_cast<const char *>(pData) + uBytesTotal, uBytesToWrite);
		if(uBytesWritten == 0){
			MCF_THROW(Exception, ERROR_BROKEN_PIPE, Rcntws::View(L"FileOutputStream: 未能成功写入所有数据。"));
		}
		uBytesTotal += uBytesWritten;
	}
	x_u64Offset += uBytesTotal;
}
void FileOutputStream::Flush(bool bHard){
	if(bHard){
		x_vFile.Flush();
	}
}

}
