// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextInputStreamFilter.hpp"

namespace MCF {

namespace {
	void PopulateBuffer(StreamBuffer &vBuffer, Vector<char> &vecBackBuffer, AbstractInputStream *pStream){
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
				uBytesRead = pStream->Peek(vecBackBuffer.GetData(), vecBackBuffer.GetSize());
				if((uBytesRead > 0) && (vecBackBuffer[uBytesRead - 1] == '\r')){
					--uBytesRead;
				}
				pStream->Discard(uBytesRead);
				vecBackBuffer.Pop(vecBackBuffer.GetSize() - uBytesRead);
			} catch(...){
				vecBackBuffer.Clear();
				throw;
			}
			if(uBytesRead == 0){
				break;
			}
			auto pchLineBegin = vecBackBuffer.GetData();
			const auto pchEnd = pchLineBegin + vecBackBuffer.GetSize();
			for(;;){
				auto pchLineEnd = static_cast<char *>(std::memchr(pchLineBegin, '\n', static_cast<std::size_t>(pchEnd - pchLineBegin)));
				if(!pchLineEnd){
					break;
				}
				if((pchLineEnd != pchLineBegin) && (pchLineEnd[-1] == '\r')){
					std::memmove(pchLineEnd - 1, pchLineEnd, static_cast<std::size_t>(pchEnd - pchLineEnd));
					--pchLineEnd;
					vecBackBuffer.Pop();
				}
				pchLineBegin = pchLineEnd + 1;
			}
		}
	}
}

TextInputStreamFilter::~TextInputStreamFilter(){
}

int TextInputStreamFilter::Peek() const {
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Peek();
}
int TextInputStreamFilter::Get(){
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Get();
}
bool TextInputStreamFilter::Discard(){
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Discard();
}

std::size_t TextInputStreamFilter::Peek(void *pData, std::size_t uSize) const {
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Peek(pData, uSize);
}
std::size_t TextInputStreamFilter::Get(void *pData, std::size_t uSize){
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Get(pData, uSize);
}
std::size_t TextInputStreamFilter::Discard(std::size_t uSize){
	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_pUnderlyingStream.Get());
	return x_vBuffer.Discard(uSize);
}

}
