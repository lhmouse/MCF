// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "FileInputStream.hpp"
#include "../Core/MinMax.hpp"

namespace MCF {

FileInputStream::~FileInputStream(){
}

std::size_t FileInputStream::X_ReadFromCurrentOffset(void *pData, std::size_t uSize){
	const auto pbyData = static_cast<unsigned char *>(pData);
	std::size_t uBytesTotal = 0;
	for(;;){
		auto uBytesToRead = Min(uSize - uBytesTotal, UINT32_MAX);
		if(uBytesToRead == 0){
			break;
		}
		const auto uBytesRead = x_vFile.Read(pbyData + uBytesTotal, uBytesToRead, x_u64Offset + uBytesTotal);
		uBytesTotal += uBytesRead;
		if(uBytesRead < uBytesToRead){
			break;
		}
	}
	return uBytesTotal;
}
std::size_t FileInputStream::X_DiscardFromCurrentOffset(std::size_t uSize){
	std::size_t uBytesTotal = 0;
	const auto u64FileSize = x_vFile.GetSize();
	if(x_u64Offset < u64FileSize){
		uBytesTotal = static_cast<std::size_t>(Min(uSize, u64FileSize - x_u64Offset));
	}
	return uBytesTotal;
}

int FileInputStream::Peek(){
	int nRet = -1;
	unsigned char byData;
	const auto uBytesRead = X_ReadFromCurrentOffset(&byData, 1);
	if(uBytesRead >= 1){
		nRet = byData;
	}
	return nRet;
}
int FileInputStream::Get(){
	int nRet = -1;
	unsigned char byData;
	const auto uBytesRead = X_ReadFromCurrentOffset(&byData, 1);
	if(uBytesRead >= 1){
		nRet = byData;
	}
	x_u64Offset += uBytesRead;
	return nRet;
}
bool FileInputStream::Discard(){
	const auto uBytesDiscarded = X_DiscardFromCurrentOffset(1);
	x_u64Offset += uBytesDiscarded;
	return uBytesDiscarded >= 1;
}
std::size_t FileInputStream::Peek(void *pData, std::size_t uSize){
	const auto uBytesRead = X_ReadFromCurrentOffset(pData, uSize);
	return uBytesRead;
}
std::size_t FileInputStream::Get(void *pData, std::size_t uSize){
	const auto uBytesRead = X_ReadFromCurrentOffset(pData, uSize);
	x_u64Offset += uBytesRead;
	return uBytesRead;
}
std::size_t FileInputStream::Discard(std::size_t uSize){
	const auto uBytesDiscarded = X_DiscardFromCurrentOffset(uSize);
	x_u64Offset += uBytesDiscarded;
	return uBytesDiscarded;
}

}
