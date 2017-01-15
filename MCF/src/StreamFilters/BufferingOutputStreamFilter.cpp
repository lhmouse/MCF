// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "BufferingOutputStreamFilter.hpp"
#include "../Core/MinMax.hpp"

namespace MCF {

enum : std::size_t {
	kFlushSizeThreshold = 1024,
};

BufferingOutputStreamFilter::~BufferingOutputStreamFilter(){
	try {
		BufferingOutputStreamFilter::Flush(false);
	} catch(...){
	}
}

void BufferingOutputStreamFilter::Put(unsigned char byData){
	BufferingOutputStreamFilter::Put(&byData, 1);
}
void BufferingOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	if(x_uCapacity - x_uSize < uSize){
		const auto uNewCapacity = Max(x_uSize + uSize, kFlushSizeThreshold * 2);
		UniquePtr<unsigned char []> pbyNewBuffer(new unsigned char[uNewCapacity]);
		if(x_uSize > 0){
			std::memcpy(pbyNewBuffer.Get(), x_pbyBuffer.Get(), x_uSize);
		}
		x_pbyBuffer = std::move(pbyNewBuffer);
		x_uCapacity = uNewCapacity;
	}
	MCF_ASSERT(x_uCapacity - x_uSize >= uSize);
	if(uSize > 0){
		std::memcpy(x_pbyBuffer.Get() + x_uSize, pData, uSize);
		x_uSize += uSize;
	}

	if(x_uSize > kFlushSizeThreshold){
		GetUnderlyingStream()->Put(x_pbyBuffer.Get(), x_uSize);
		x_uSize = 0;
	}
}
void BufferingOutputStreamFilter::Flush(bool bHard){
	if(x_uSize > 0){
		GetUnderlyingStream()->Put(x_pbyBuffer.Get(), x_uSize);
		x_uSize = 0;
	}

	GetUnderlyingStream()->Flush(bHard);
}

}
