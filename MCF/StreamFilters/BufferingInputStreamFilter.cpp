// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "BufferingInputStreamFilter.hpp"

namespace MCF {

namespace {
	constexpr std::size_t kStepSize = 4096;

	void PopulateBuffer(StreamBuffer &vBuffer, Vector<unsigned char> &vecBackBuffer, AbstractInputStream *pStream){
		for(;;){
			if(!vecBackBuffer.IsEmpty()){
				auto uBytesToPut = vecBackBuffer.GetSize();
				vBuffer.Put(vecBackBuffer.GetData(), uBytesToPut);
				// CopyBackward(vecBackBuffer.GetEnd() - uBytesToPut, vecBackBuffer.GetBegin() + uBytesToPut, vecBackBuffer.GetEnd());
				vecBackBuffer.Pop(uBytesToPut);
			}
			if(!vBuffer.IsEmpty()){
				break;
			}

			std::size_t uBytesRead;
			const auto pbyStepBuffer = vecBackBuffer.ResizeMore(kStepSize);
			try {
				uBytesRead = pStream->Get(pbyStepBuffer, kStepSize);
			} catch(...){
				vecBackBuffer.Pop(kStepSize);
				throw;
			}
			vecBackBuffer.Pop(kStepSize - uBytesRead);

			if(uBytesRead == 0){
				break;
			}
		}
	}
}

BufferingInputStreamFilter::~BufferingInputStreamFilter(){
}

int BufferingInputStreamFilter::Peek() const {
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Peek();
}
int BufferingInputStreamFilter::Get(){
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Get();
}
bool BufferingInputStreamFilter::Discard(){
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Discard();
}

std::size_t BufferingInputStreamFilter::Peek(void *pData, std::size_t uSize) const {
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Peek(pData, uSize);
}
std::size_t BufferingInputStreamFilter::Get(void *pData, std::size_t uSize){
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Get(pData, uSize);
}
std::size_t BufferingInputStreamFilter::Discard(std::size_t uSize){
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Discard(uSize);
}

}
