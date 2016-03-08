// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_BufferedOutputStream.hpp"

namespace MCF {

namespace Impl_BufferedOutputStream {
	BufferedOutputStream::~BufferedOutputStream(){
		try {
			Flush(kFlushBufferAll);
		} catch(...){
		}
	}

	void BufferedOutputStream::Flush(BufferedOutputStream::FlushLevel eLevel){
		enum : std::size_t {
			 kStepSize = 4096,
		};

		bool bNoMoreAvail = false;
		for(;;){
			if(!x_vecBackBuffer.IsEmpty()){
				x_pUnderlyingStream->Put(x_vecBackBuffer.GetData(), x_vecBackBuffer.GetSize());
				x_vecBackBuffer.Clear();
			}
			if(bNoMoreAvail){
				break;
			}
			if((eLevel < kFlushBufferAll) && (x_sbufFrontBuffer.GetSize() < kStepSize)){
				break;
			}

			std::size_t uBytesToWrite;
			const auto pbyStepBuffer = x_vecBackBuffer.ResizeMore(kStepSize);
			try {
				uBytesToWrite = x_sbufFrontBuffer.Get(pbyStepBuffer, kStepSize);
			} catch(...){
				x_vecBackBuffer.Pop(kStepSize);
				throw;
			}
			x_vecBackBuffer.Pop(kStepSize - uBytesToWrite);

			if(uBytesToWrite < kStepSize){
				bNoMoreAvail = true;
			}
		}

		if(eLevel >= kFlushStreamSoft){
			x_pUnderlyingStream->Flush(eLevel >= kFlushStreamHard);
		}
	}
}

}
