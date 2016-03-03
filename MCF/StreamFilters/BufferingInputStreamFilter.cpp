// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "BufferingInputStreamFilter.hpp"

namespace MCF {

namespace {
	void PopulateBuffer(StreamBuffer &vBuffer, Vector<unsigned char> &vecBackBuffer, AbstractInputStream *pStream){
		for(;;){
			if(!vecBackBuffer.IsEmpty()){
				vBuffer.Put(vecBackBuffer.GetData(), vecBackBuffer.GetSize());
				vecBackBuffer.Clear();
			}
			if(!vBuffer.IsEmpty()){
				break;
			}

			std::size_t uBytesRead;
			vecBackBuffer.Resize(4096);
			try {
				uBytesRead = pStream->Get(vecBackBuffer.GetData(), vecBackBuffer.GetSize());
				vecBackBuffer.Pop(vecBackBuffer.GetSize() - uBytesRead);
			} catch(...){
				vecBackBuffer.Clear();
				throw;
			}
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
