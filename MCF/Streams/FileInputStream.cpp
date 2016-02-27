// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "FileInputStream.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

namespace {
	std::size_t RealRead(const File &vFile, void *pData, std::size_t uSize, std::uint64_t u64Offset){
		const auto pbyData = static_cast<unsigned char *>(pData);
		std::size_t uBytesTotal = 0;
		for(;;){
			auto uBytesToRead = Min(uSize - uBytesTotal, UINT32_MAX);
			if(uBytesToRead == 0){
				break;
			}
			const auto uBytesRead = vFile.Read(pbyData + uBytesTotal, uBytesToRead, u64Offset + uBytesTotal);
			if(uBytesRead == 0){
				break;
			}
			uBytesTotal += uBytesRead;
		}
		return uBytesTotal;
	}
	std::size_t RealDiscard(const File &vFile, std::size_t uSize, std::uint64_t u64Offset){
		std::size_t uBytesTotal = 0;
		const auto u64FileSize = vFile.GetSize();
		if(u64Offset < u64FileSize){
			uBytesTotal = static_cast<std::size_t>(Min(uSize, u64FileSize - u64Offset));
		}
		return uBytesTotal;
	}
}

FileInputStream::~FileInputStream(){
}

int FileInputStream::Peek() const {
	int nRet = -1;
	unsigned char byData;
	const auto uBytesRead = RealRead(x_vFile, &byData, 1, x_u64Offset);
	if(uBytesRead >= 1){
		nRet = byData;
	}
	return nRet;
}
int FileInputStream::Get(){
	int nRet = -1;
	unsigned char byData;
	const auto uBytesRead = RealRead(x_vFile, &byData, 1, x_u64Offset);
	if(uBytesRead >= 1){
		nRet = byData;
	}
	x_u64Offset += uBytesRead;
	return nRet;
}
bool FileInputStream::Discard(){
	const auto uBytesDiscarded = RealDiscard(x_vFile, 1, x_u64Offset);
	x_u64Offset += uBytesDiscarded;
	return uBytesDiscarded >= 1;
}

std::size_t FileInputStream::Peek(void *pData, std::size_t uSize) const {
	const auto uBytesRead = RealRead(x_vFile, pData, uSize, x_u64Offset);
	return uBytesRead;
}
std::size_t FileInputStream::Get(void *pData, std::size_t uSize){
	const auto uBytesRead = RealRead(x_vFile, pData, uSize, x_u64Offset);
	x_u64Offset += uBytesRead;
	return uBytesRead;
}
std::size_t FileInputStream::Discard(std::size_t uSize){
	const auto uBytesDiscarded = RealDiscard(x_vFile, uSize, x_u64Offset);
	x_u64Offset += uBytesDiscarded;
	return uBytesDiscarded;
}

}
