// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextInputStreamFilter.hpp"
#include "../Utilities/CopyMoveFill.hpp"

namespace MCF {

namespace {
	constexpr std::size_t kStepSize = 256;

	void PopulateBuffer(StreamBuffer &vBuffer, Vector<char> &vecBackBuffer, AbstractInputStream *pStream){
		for(;;){
			if(!vecBackBuffer.IsEmpty()){
				if(vecBackBuffer.GetEnd()[-1] == '\r'){
					vBuffer.Put(vecBackBuffer.GetData(), vecBackBuffer.GetSize() - 1);
					vecBackBuffer.Clear();
					vecBackBuffer.UncheckedPush('\r');
				} else {
					vBuffer.Put(vecBackBuffer.GetData(), vecBackBuffer.GetSize());
					vecBackBuffer.Clear();
				}
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

			auto pchLineBegin = vecBackBuffer.GetBegin();
			for(;;){
				auto pchLineEnd = static_cast<char *>(std::memchr(pchLineBegin, '\n', static_cast<std::size_t>(vecBackBuffer.GetEnd() - pchLineBegin)));
				if(!pchLineEnd){
					break;
				}
				if((pchLineEnd != pchLineBegin) && (pchLineEnd[-1] == '\r')){
					Copy(pchLineEnd - 1, pchLineEnd, vecBackBuffer.GetEnd());
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
