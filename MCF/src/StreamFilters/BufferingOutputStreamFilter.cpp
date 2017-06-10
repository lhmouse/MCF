// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "BufferingOutputStreamFilter.hpp"
#include "../Core/MinMax.hpp"

namespace MCF {

enum : std::size_t {
	kFlushThreshold = 1024,
};

BufferingOutputStreamFilter::~BufferingOutputStreamFilter(){
	try {
		BufferingOutputStreamFilter::Flush(false);
	} catch(...){ }
}

void BufferingOutputStreamFilter::Put(unsigned char byData){
	BufferingOutputStreamFilter::Put(&byData, 1);
}
void BufferingOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	if(uSize > 0){
		x_vecBuffer.Reserve(Max(uSize, kFlushThreshold * 2));
		const auto pbyBuffer = x_vecBuffer.ResizeMore(uSize);
		std::memcpy(pbyBuffer, pData, uSize);
	}

	if(x_vecBuffer.GetSize() > kFlushThreshold){
		GetUnderlyingStream()->Put(x_vecBuffer.GetData(), x_vecBuffer.GetSize());
		x_vecBuffer.Clear();
	}
}
void BufferingOutputStreamFilter::Flush(bool bHard){
	if(x_vecBuffer.GetSize() > 0){
		GetUnderlyingStream()->Put(x_vecBuffer.GetData(), x_vecBuffer.GetSize());
		x_vecBuffer.Clear();
	}

	GetUnderlyingStream()->Flush(bHard);
}

}
