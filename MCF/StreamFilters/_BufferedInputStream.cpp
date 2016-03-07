// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_BufferedInputStream.hpp"

namespace MCF {

namespace Impl_BufferedInputStream {
	enum : std::size_t {
		 kStepSize = 4096,
	};

	BufferedInputStream::~BufferedInputStream(){
	}

	void BufferedInputStream::X_PopulateTempBuffer(std::size_t uExpected){
		bool bNoMoreAvail = false;
		for(;;){
			if(!x_vecBackBuffer.IsEmpty()){
				x_sbufBufferedData.Put(x_vecBackBuffer.GetData(), x_vecBackBuffer.GetSize());
				x_vecBackBuffer.Clear();
			}
			if(bNoMoreAvail || (x_sbufBufferedData.GetSize() >= uExpected)){
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

	int BufferedInputStream::Peek(){
		X_PopulateTempBuffer(1);
		return x_sbufBufferedData.Peek();
	}
	int BufferedInputStream::Get(){
		X_PopulateTempBuffer(1);
		return x_sbufBufferedData.Get();
	}
	bool BufferedInputStream::Discard(){
		X_PopulateTempBuffer(1);
		return x_sbufBufferedData.Discard();
	}
	void BufferedInputStream::UncheckedDiscard() noexcept {
		ASSERT(!x_sbufBufferedData.IsEmpty());

		x_sbufBufferedData.Discard();
	}

	std::size_t BufferedInputStream::Peek(void *pData, std::size_t uSize){
		X_PopulateTempBuffer(uSize);
		return x_sbufBufferedData.Peek(pData, uSize);
	}
	std::size_t BufferedInputStream::Get(void *pData, std::size_t uSize){
		X_PopulateTempBuffer(uSize);
		return x_sbufBufferedData.Get(pData, uSize);
	}
	std::size_t BufferedInputStream::Discard(std::size_t uSize){
		X_PopulateTempBuffer(uSize);
		return x_sbufBufferedData.Discard(uSize);
	}
	void BufferedInputStream::UncheckedDiscard(std::size_t uSize) noexcept {
		ASSERT(x_sbufBufferedData.GetSize() >= uSize);

		x_sbufBufferedData.Discard(uSize);
	}
}

}
