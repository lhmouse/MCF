// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "BufferingOutputStreamFilter.hpp"

namespace MCF {

namespace {
	void FlushBuffer(AbstractOutputStream *pStream, StreamBuffer &vBuffer, std::size_t uThreshold){
		for(;;){
			if(vBuffer.GetSize() < uThreshold){
				break;
			}
			unsigned char abyBackBuffer[4096];
			const auto uBytesToWrite = vBuffer.Peek(abyBackBuffer, sizeof(abyBackBuffer));
			if(uBytesToWrite == 0){
				break;
			}
			pStream->Put(abyBackBuffer, uBytesToWrite);
			vBuffer.Discard(uBytesToWrite);
		}
	}
}

BufferingOutputStreamFilter::~BufferingOutputStreamFilter(){
	try {
		FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, 0);
	} catch(...){
	}
}

void BufferingOutputStreamFilter::Put(unsigned char byData){
	x_vBuffer.Put(byData);
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, 4096);
}

void BufferingOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	x_vBuffer.Put(pData, uSize);
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, 4096);
}

void BufferingOutputStreamFilter::Flush(bool bHard){
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, 0);

	x_pUnderlyingStream->Flush(bHard);
}

}
