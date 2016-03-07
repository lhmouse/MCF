// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_BufferedOutputStream.hpp"

namespace MCF {

namespace Impl_BufferedOutputStream {
	namespace {
		enum : std::size_t {
			 kStepSize = 4096,
		};

		void FlushTempBuffer(AbstractOutputStream *pStream, StreamBuffer &vBuffer, Vector<unsigned char> &vecBackBuffer, std::size_t uThreshold){
			bool bNoMoreAvail = false;
			for(;;){
				if(!vecBackBuffer.IsEmpty()){
					pStream->Put(vecBackBuffer.GetData(), vecBackBuffer.GetSize());
					vecBackBuffer.Clear();
				}
				if(bNoMoreAvail || (vBuffer.GetSize() < uThreshold)){
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

				if(uBytesToWrite < kStepSize){
					bNoMoreAvail = true;
				}
			}
		}
	}

	BufferedOutputStream::~BufferedOutputStream(){
		try {
			FlushTempBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer, 0);
		} catch(...){
		}
	}

	void BufferedOutputStream::Put(unsigned char byData){
		x_vBuffer.Put(byData);
		FlushTempBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer, kStepSize);
	}

	void BufferedOutputStream::Put(const void *pData, std::size_t uSize){
		x_vBuffer.Put(pData, uSize);
		FlushTempBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer, kStepSize);
	}

	void BufferedOutputStream::Splice(StreamBuffer &vBuffer){
		x_vBuffer.Splice(vBuffer);
		FlushTempBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer, kStepSize);
	}

	void BufferedOutputStream::Flush(bool bHard){
		FlushTempBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer, 0);

		x_pUnderlyingStream->Flush(bHard);
	}
}

}
