// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextOutputStreamFilter.hpp"

namespace MCF {

namespace {
	void FlushBuffer(AbstractOutputStream *pStream, StreamBuffer &vBuffer){
		for(;;){
			unsigned char abyBackBuffer[4096];
			const auto uBytesToWrite = vBuffer.Peek(abyBackBuffer, sizeof(abyBackBuffer));
			if(uBytesToWrite == 0){
				break;
			}

			pStream->Put(abyBackBuffer, uBytesToWrite);
			vBuffer.Discard(uBytesToWrite);
		}
	}
}

TextOutputStreamFilter::~TextOutputStreamFilter(){
}

void TextOutputStreamFilter::Put(unsigned char byData){
	if(byData != '\n'){
		x_vBuffer.Put(byData);
	} else {
		x_vBuffer.Put("\r\n", 2);
	}
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer);
}

void TextOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	StreamBuffer vNewBuffer;
	auto pchLineBegin = static_cast<const char *>(pData);
	const auto pchEnd = pchLineBegin + uSize;
	for(;;){
		const auto pchLineEnd = static_cast<char *>(std::memchr(pchLineBegin, '\n', static_cast<std::size_t>(pchEnd - pchLineBegin)));
		if(!pchLineEnd){
			vNewBuffer.Put(pchLineBegin, static_cast<std::size_t>(pchEnd - pchLineBegin));
			break;
		}
		vNewBuffer.Put(pchLineBegin, static_cast<std::size_t>(pchLineEnd - pchLineBegin));
		vNewBuffer.Put("\r\n", 2);
		pchLineBegin = pchLineEnd + 1;
	}
	x_vBuffer.Splice(vNewBuffer);
	FlushBuffer(x_pUnderlyingStream.Get(), x_vBuffer);
}

void TextOutputStreamFilter::Flush(bool bHard){
	x_pUnderlyingStream->Flush(bHard);
}

}
