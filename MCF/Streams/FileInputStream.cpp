// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "FileInputStream.hpp"

namespace MCF {

FileInputStream::~FileInputStream(){
}

int FileInputStream::Peek() const {
	int nRet = -1;
	unsigned char byData;
	if(Peek(&byData, 1) >= 1){
		nRet = byData;
	}
	return nRet;
}
int FileInputStream::Get(){
	int nRet = -1;
	unsigned char byData;
	if(Get(&byData, 1) >= 1){
		nRet = byData;
	}
	return nRet;
}
bool FileInputStream::Discard(){
	return Discard(1) >= 1;
}

std::size_t FileInputStream::Peek(void *pData, std::size_t uSize) const {
	const auto pbyData = static_cast<unsigned char *>(pData);
	std::size_t uBytesTotal = 0;
	for(;;){
		auto uBytesToRead = uSize - uBytesTotal;
		if(uBytesToRead == 0){
			break;
		}
		if(uBytesToRead > UINT32_MAX){
			uBytesToRead = UINT32_MAX;
		}
		const auto uBytesRead = x_vFile.Read(pbyData + uBytesTotal, uBytesToRead, x_u64Offset + uBytesTotal);
		if(uBytesRead == 0){
			break;
		}
		uBytesTotal += uBytesRead;
	}
	return uBytesTotal;
}
std::size_t FileInputStream::Get(void *pData, std::size_t uSize){
	const auto pbyData = static_cast<unsigned char *>(pData);
	std::size_t uBytesTotal = 0;
	for(;;){
		auto uBytesToRead = uSize - uBytesTotal;
		if(uBytesToRead == 0){
			break;
		}
		if(uBytesToRead > UINT32_MAX){
			uBytesToRead = UINT32_MAX;
		}
		const auto uBytesRead = x_vFile.Read(pbyData + uBytesTotal, uBytesToRead, x_u64Offset + uBytesTotal);
		if(uBytesRead == 0){
			break;
		}
		uBytesTotal += uBytesRead;
	}
	x_u64Offset += uBytesTotal;
	return uBytesTotal;
}
std::size_t FileInputStream::Discard(std::size_t uSize){
	std::size_t uBytesTotal = 0;
	const auto u64FileSize = x_vFile.GetSize();
	if(x_u64Offset < u64FileSize){
		auto u64BytesDiscarded = u64FileSize - x_u64Offset;
		if(u64BytesDiscarded > uSize){
			u64BytesDiscarded = uSize;
		}
		uBytesTotal = static_cast<std::size_t>(u64BytesDiscarded);
	}
	x_u64Offset += uBytesTotal;
	return uBytesTotal;
}

}
