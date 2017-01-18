// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "BufferingInputStreamFilter.hpp"
#include "../Core/MinMax.hpp"

namespace MCF {

enum : std::size_t {
	kPopulationThreshold = 1024,
};

BufferingInputStreamFilter::~BufferingInputStreamFilter(){
	try {
		BufferingInputStreamFilter::Invalidate();
	} catch(...){
	}
}

int BufferingInputStreamFilter::Peek(){
	int nRet = -1;
	unsigned char byData;
	if(BufferingInputStreamFilter::Peek(&byData, 1) >= 1){
		nRet = byData;
	}
	return nRet;
}
int BufferingInputStreamFilter::Get(){
	int nRet = -1;
	unsigned char byData;
	if(BufferingInputStreamFilter::Get(&byData, 1) >= 1){
		nRet = byData;
	}
	return nRet;
}
bool BufferingInputStreamFilter::Discard(){
	bool bRet = false;
	if(BufferingInputStreamFilter::Discard(1) >= 1){
		bRet = true;
	}
	return bRet;
}
std::size_t BufferingInputStreamFilter::Peek(void *pData, std::size_t uSize){
	std::size_t uBytesTotal = 0;
	if(x_vecBuffer.GetSize() - x_uOffset < uSize){
		if(x_uOffset != 0){
			GetUnderlyingStream()->Discard(x_uOffset);
			x_uOffset = 0;
		}
		x_vecBuffer.Clear();
		x_vecBuffer.Reserve(Max(uSize, kPopulationThreshold * 2));
		{
			x_vecBuffer.UncheckedAppend(x_vecBuffer.GetCapacityRemaining());
			try {
				const auto uBytesRead = GetUnderlyingStream()->Peek(x_vecBuffer.GetData(), x_vecBuffer.GetSize());
				x_vecBuffer.Pop(x_vecBuffer.GetSize() - uBytesRead);
			} catch(...){
				x_vecBuffer.Clear();
				throw;
			}
		}
		const auto uBytesCopied = Min(uSize, x_vecBuffer.GetSize());
		if(uBytesCopied > 0){
			std::memcpy(pData, x_vecBuffer.GetData(), uBytesCopied);
		}
		uBytesTotal += uBytesCopied;
	} else {
		const auto uBytesCopied = uSize;
		if(uBytesCopied > 0){
			std::memcpy(pData, x_vecBuffer.GetData() + x_uOffset, uBytesCopied);
		}
		uBytesTotal += uBytesCopied;
	}
	return uBytesTotal;
}
std::size_t BufferingInputStreamFilter::Get(void *pData, std::size_t uSize){
	const auto uBytesTotal = BufferingInputStreamFilter::Peek(pData, uSize);
	x_uOffset += uBytesTotal;
	return uBytesTotal;
}
std::size_t BufferingInputStreamFilter::Discard(std::size_t uSize){
	std::size_t uBytesTotal = 0;
	if(x_vecBuffer.GetSize() - x_uOffset < uSize){
		std::size_t uBytesDiscarded;
		if(x_uOffset != 0){
			uBytesDiscarded = GetUnderlyingStream()->Discard(x_uOffset + uSize) - x_uOffset;
			x_uOffset = 0;
		} else {
			uBytesDiscarded = GetUnderlyingStream()->Discard(uSize);
		}
		x_vecBuffer.Clear();
		uBytesTotal += uBytesDiscarded;
	} else {
		const auto uBytesDiscarded = uSize;
		uBytesTotal += uBytesDiscarded;
		x_uOffset += uBytesTotal;
	}
	return uBytesTotal;
}
void BufferingInputStreamFilter::Invalidate(){
	if(x_uOffset != 0){
		GetUnderlyingStream()->Discard(x_uOffset);
		x_uOffset = 0;
	}
	x_vecBuffer.Clear();

	GetUnderlyingStream()->Invalidate();
}

}
