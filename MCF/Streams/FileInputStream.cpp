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
	auto uBytesRead = x_vFile.Read(pData, uSize, x_u64Offset);
	return uBytesRead;
}
std::size_t FileInputStream::Get(void *pData, std::size_t uSize){
	auto uBytesRead = x_vFile.Read(pData, uSize, x_u64Offset);
	x_u64Offset += uBytesRead;
	return uBytesRead;
}
std::size_t FileInputStream::Discard(std::size_t uSize){
	auto uBytesDiscarded = (std::size_t)0;
	const auto u64FileSize = x_vFile.GetSize();
	if(x_u64Offset < u64FileSize){
		uBytesDiscarded = uSize;
		const auto uRemaining = u64FileSize - x_u64Offset;
		if(uBytesDiscarded > uRemaining){
			uBytesDiscarded = (std::size_t)uRemaining;
		}
		x_u64Offset += uBytesDiscarded;
	}
	return uBytesDiscarded;
}

}
