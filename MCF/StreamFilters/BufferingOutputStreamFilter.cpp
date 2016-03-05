// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "BufferingOutputStreamFilter.hpp"

namespace MCF {

namespace {
	constexpr std::size_t kStepSize = 4096;

	void FlushBuffer(AbstractOutputStream *pStream, StreamBuffer &vBuffer, Vector<unsigned char> &vecBackBuffer, std::size_t uThreshold){
		for(;;){
			if(!vecBackBuffer.IsEmpty()){
				const auto pbyToWriteBegin = vecBackBuffer.GetData();
				const auto pbyToWriteEnd   = pbyToWriteBegin + vecBackBuffer.GetSize();
				pStream->Put(pbyToWriteBegin, static_cast<std::size_t>(pbyToWriteEnd - pbyToWriteBegin));
				vecBackBuffer.Erase(pbyToWriteBegin, pbyToWriteEnd);
			}
			if(vBuffer.GetSize() < uThreshold){
				break;
			}

			std::size_t uBytesToWrite;
			const auto pbyStepBuffer = vecBackBuffer.ResizeMore(kStepSize);
			try {
				uBytesToWrite = vBuffer.Get(pbyStepBuffer, kStepSize);
			} catch(...){
				vecBackBuffer.Pop(kStepSize);
				throw;
			}
			vecBackBuffer.Pop(kStepSize - uBytesToWrite);

			if(uBytesToWrite == 0){
				break;
			}
		}
	}
}

BufferingOutputStreamFilter::~BufferingOutputStreamFilter(){
	try {
		FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer, 0);
	} catch(...){
	}
}

void BufferingOutputStreamFilter::Put(unsigned char byData){
	x_vBuffer.Put(byData);
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer, 4096);
}

void BufferingOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	x_vBuffer.Put(pData, uSize);
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer, 4096);
}

void BufferingOutputStreamFilter::Flush(bool bHard){
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer, 0);

	x_pUnderlyingStream->Flush(bHard);
}

}
