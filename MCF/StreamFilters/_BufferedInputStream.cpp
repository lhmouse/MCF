// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_BufferedInputStream.hpp"

namespace MCF {

namespace Impl_BufferedInputStream {
	BufferedInputStream::~BufferedInputStream(){
	}

	void BufferedInputStream::PopulateBuffer(std::size_t uExpected){
		enum : std::size_t {
			 kStepSize = 4096,
		};

		bool bNoMoreAvail = false;
		for(;;){
			if(!x_vecBackBuffer.IsEmpty()){
				x_sbufFrontBuffer.Put(x_vecBackBuffer.GetData(), x_vecBackBuffer.GetSize());
				x_vecBackBuffer.Clear();
			}
			if(bNoMoreAvail){
				break;
			}
			if(x_sbufFrontBuffer.GetSize() >= uExpected){
				break;
			}

			std::size_t uBytesRead;
			const auto pbyStepBuffer = x_vecBackBuffer.ResizeMore(kStepSize);
			try {
				uBytesRead = x_pUnderlyingStream->Get(pbyStepBuffer, kStepSize);
			} catch(...){
				x_vecBackBuffer.Pop(kStepSize);
				throw;
			}
			x_vecBackBuffer.Pop(kStepSize - uBytesRead);

			if(uBytesRead < kStepSize){
				bNoMoreAvail = true;
			}
		}
	}
}

}
