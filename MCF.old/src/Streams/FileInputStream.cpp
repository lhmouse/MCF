// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "FileInputStream.hpp"
#include "../Core/MinMax.hpp"

namespace MCF {

FileInputStream::~FileInputStream(){ }

int FileInputStream::Peek(){
	int nRet = -1;
	unsigned char byData;
	if(FileInputStream::Peek(&byData, 1) >= 1){
		nRet = byData;
	}
	return nRet;
}
int FileInputStream::Get(){
	int nRet = -1;
	unsigned char byData;
	if(FileInputStream::Get(&byData, 1) >= 1){
		nRet = byData;
	}
	return nRet;
}
bool FileInputStream::Discard(){
	bool bRet = false;
	if(FileInputStream::Discard(1) >= 1){
		bRet = true;
	}
	return bRet;
}
std::size_t FileInputStream::Peek(void *pData, std::size_t uSize){
	std::size_t uBytesTotal = 0;
	for(;;){
		const auto uBytesToRead = uSize - uBytesTotal;
		if(uBytesToRead == 0){
			break;
		}
		const auto uBytesRead = x_vFile.Read(static_cast<char *>(pData) + uBytesTotal, uBytesToRead, x_u64Offset + uBytesTotal);
		if(uBytesRead == 0){
			break;
		}
		uBytesTotal += uBytesRead;
	}
	return uBytesTotal;
}
std::size_t FileInputStream::Get(void *pData, std::size_t uSize){
	const auto uBytesTotal = FileInputStream::Peek(pData, uSize);
	x_u64Offset += uBytesTotal;
	return uBytesTotal;
}
std::size_t FileInputStream::Discard(std::size_t uSize){
	std::size_t uBytesTotal = 0;
	const auto u64FileSize = x_vFile.GetSize();
	if(x_u64Offset < u64FileSize){
		const auto uBytesDiscarded = static_cast<std::size_t>(Min(uSize, u64FileSize - x_u64Offset));
		uBytesTotal += uBytesDiscarded;
	}
	x_u64Offset += uBytesTotal;
	return uBytesTotal;
}
void FileInputStream::Invalidate(){ }

}
