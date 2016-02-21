// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "BufferingInputStreamFilter.hpp"

namespace MCF {

namespace {
	void PopulateBuffer(StreamBuffer &vBuffer, std::size_t uSizeExpected, AbstractInputStream &vStream){
		for(;;){
			if(vBuffer.GetSize() >= uSizeExpected){
				break;
			}
			unsigned char abyTemp[4096];
			const auto uBytesRead = vStream.Peek(abyTemp, sizeof(abyTemp));
			if(uBytesRead == 0){
				break;
			}
			auto vNewData = StreamBuffer(abyTemp, uBytesRead);
			vStream.Discard(uBytesRead);
			vBuffer.Splice(vNewData);
		}
	}
}

BufferingInputStreamFilter::~BufferingInputStreamFilter(){
}

int BufferingInputStreamFilter::Peek() const {
	PopulateBuffer(x_vBuffer, 1, *x_pUnderlyingStream);
	return x_vBuffer.Peek();
}
int BufferingInputStreamFilter::Get(){
	PopulateBuffer(x_vBuffer, 1, *x_pUnderlyingStream);
	return x_vBuffer.Get();
}
bool BufferingInputStreamFilter::Discard(){
	PopulateBuffer(x_vBuffer, 1, *x_pUnderlyingStream);
	return x_vBuffer.Discard();
}

std::size_t BufferingInputStreamFilter::Peek(void *pData, std::size_t uSize) const {
	PopulateBuffer(x_vBuffer, uSize, *x_pUnderlyingStream);
	return x_vBuffer.Peek(pData, uSize);
}
std::size_t BufferingInputStreamFilter::Get(void *pData, std::size_t uSize){
	PopulateBuffer(x_vBuffer, uSize, *x_pUnderlyingStream);
	return x_vBuffer.Get(pData, uSize);
}
std::size_t BufferingInputStreamFilter::Discard(std::size_t uSize){
	PopulateBuffer(x_vBuffer, uSize, *x_pUnderlyingStream);
	return x_vBuffer.Discard(uSize);
}

}
