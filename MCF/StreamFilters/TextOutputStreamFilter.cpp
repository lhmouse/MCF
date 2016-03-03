// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextOutputStreamFilter.hpp"

namespace MCF {

namespace {
	void FlushBuffer(AbstractOutputStream *pStream, StreamBuffer &vBuffer, Vector<char> &vecBackBuffer){
		for(;;){
			if(!vecBackBuffer.IsEmpty()){
				pStream->Put(vecBackBuffer.GetData(), vecBackBuffer.GetSize());
				vecBackBuffer.Clear();
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

			auto pchLineBegin = vecBackBuffer.GetBegin();
			for(;;){
				auto pchLineEnd = static_cast<char *>(std::memchr(pchLineBegin, '\n', static_cast<std::size_t>(vecBackBuffer.GetEnd() - pchLineBegin)));
				if(!pchLineEnd){
					break;
				}
				pchLineEnd = vecBackBuffer.Emplace(pchLineEnd, '\r') + 1;
				pchLineBegin = pchLineEnd + 1;
			}
		}
	}
}

TextOutputStreamFilter::~TextOutputStreamFilter(){
	try {
		FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer);
	} catch(...){
	}
}

void TextOutputStreamFilter::Put(unsigned char byData){
	x_vBuffer.Put(byData);
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer);
}

void TextOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	x_vBuffer.Put(pData, uSize);
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer);
}

void TextOutputStreamFilter::Flush(bool bHard){
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer, x_vecBackBuffer);

	x_pUnderlyingStream->Flush(bHard);
}

}
