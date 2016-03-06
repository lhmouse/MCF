// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextOutputStreamFilter.hpp"
#include "../Utilities/CopyMoveFill.hpp"

namespace MCF {

namespace {
	constexpr std::size_t kStepSize = 256;

	void FlushBuffer(AbstractOutputStream *pStream, StreamBuffer &vBuffer, Vector<char> &vecBackBuffer){
		for(;;){
			if(!vecBackBuffer.IsEmpty()){
				auto uBytesToPut = vecBackBuffer.GetSize();
				pStream->Put(vecBackBuffer.GetData(), uBytesToPut);
				// CopyBackward(vecBackBuffer.GetEnd() - uBytesToPut, vecBackBuffer.GetBegin() + uBytesToPut, vecBackBuffer.GetEnd());
				vecBackBuffer.Pop(uBytesToPut);
			}

			std::size_t uBytesToWrite;
			vecBackBuffer.ReserveMore(kStepSize * 2);
			const auto pbyStepBuffer = vecBackBuffer.ResizeMore(kStepSize);
			try {
				uBytesToWrite = vBuffer.Get(pbyStepBuffer, kStepSize);
			} catch(...){
				vecBackBuffer.Pop(kStepSize);
				throw;
			}
			vecBackBuffer.Pop(kStepSize - uBytesToWrite);

			if(uBytesToWrite == 0){
				break;
			}

			auto pchLineBegin = vecBackBuffer.GetBegin();
			for(;;){
				auto pchLineEnd = static_cast<char *>(std::memchr(pchLineBegin, '\n', static_cast<std::size_t>(vecBackBuffer.GetEnd() - pchLineBegin)));
				if(!pchLineEnd){
					break;
				}
				vecBackBuffer.UncheckedPush();
				CopyBackward(vecBackBuffer.GetEnd(), pchLineEnd, vecBackBuffer.GetEnd() - 1)[-1] = '\r';
				++pchLineEnd;
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
