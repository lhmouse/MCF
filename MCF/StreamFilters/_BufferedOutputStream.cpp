// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_BufferedOutputStream.hpp"

namespace MCF {

namespace Impl_BufferedOutputStream {
	enum : std::size_t {
		 kStepSize = 4096,
	};

	BufferedOutputStream::~BufferedOutputStream(){
		try {
			X_FlushTempBuffer(0);
		} catch(...){
		}
	}

	void BufferedOutputStream::X_FlushTempBuffer(std::size_t uThreshold){
		bool bNoMoreAvail = false;
		for(;;){
			if(!x_vecBackBuffer.IsEmpty()){
				x_pUnderlyingStream->Put(x_vecBackBuffer.GetData(), x_vecBackBuffer.GetSize());
				x_vecBackBuffer.Clear();
			}
			if(bNoMoreAvail || (x_sbufBufferedData.GetSize() < uThreshold)){
				break;
			}

			std::size_t uBytesToWrite;
			const auto pbyStepBuffer = x_vecBackBuffer.ResizeMore(kStepSize);
			try {
				uBytesToWrite = x_sbufBufferedData.Get(pbyStepBuffer, kStepSize);
			} catch(...){
				x_vecBackBuffer.Pop(kStepSize);
				throw;
			}
			x_vecBackBuffer.Pop(kStepSize - uBytesToWrite);

			if(uBytesToWrite < kStepSize){
				bNoMoreAvail = true;
			}
		}
	}

	void BufferedOutputStream::Put(unsigned char byData){
		x_sbufBufferedData.Put(byData);
		X_FlushTempBuffer(kStepSize);
	}

	void BufferedOutputStream::Put(const void *pData, std::size_t uSize){
		x_sbufBufferedData.Put(pData, uSize);
		X_FlushTempBuffer(kStepSize);
	}

	void BufferedOutputStream::Flush(bool bHard){
		X_FlushTempBuffer(0);

		x_pUnderlyingStream->Flush(bHard);
	}
}

}
