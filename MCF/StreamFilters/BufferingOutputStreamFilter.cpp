// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "BufferingOutputStreamFilter.hpp"

namespace MCF {

namespace {
	void FlushBuffer(AbstractOutputStream *pStream, StreamBuffer &vBuffer, Vector<unsigned char> &vecBackBuffer, std::size_t uThreshold){
		for(;;){
			if(!vecBackBuffer.IsEmpty()){
				pStream->Put(vecBackBuffer.GetData(), vecBackBuffer.GetSize());
				vecBackBuffer.Clear();
			}
			if(vBuffer.GetSize() < uThreshold){
				break;
			}

			std::size_t uBytesToWrite;
			vecBackBuffer.Resize(4096);
			try {
				uBytesToWrite = vBuffer.Get(vecBackBuffer.GetData(), vecBackBuffer.GetSize());
				vecBackBuffer.Pop(vecBackBuffer.GetSize() - uBytesToWrite);
			} catch(...){
				vecBackBuffer.Clear();
				throw;
			}
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
