// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextOutputStreamFilter.hpp"

namespace MCF {

TextOutputStreamFilter::~TextOutputStreamFilter(){
}

void TextOutputStreamFilter::Put(unsigned char byData){
	if(byData != '\n'){
		y_vStream.Put(byData);
	} else {
		y_vStream.Put("\r\n", 2);
	}
}

void TextOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	StreamBuffer sbufNewPart;
	auto pchLineBegin = static_cast<const char *>(pData);
	const auto pchEnd = pchLineBegin + uSize;
	for(;;){
		auto pchLineEnd = static_cast<const char *>(std::memchr(pchLineBegin, '\n', static_cast<std::size_t>(pchEnd - pchLineBegin)));
		if(!pchLineEnd){
			sbufNewPart.Put(pchLineBegin, static_cast<std::size_t>(pchEnd - pchLineBegin));
			break;
		}
		sbufNewPart.Put(pchLineBegin, static_cast<std::size_t>(pchLineEnd - pchLineBegin));
		sbufNewPart.Put("\r\n", 2);
		pchLineBegin = pchLineEnd + 1;
	}
	y_vStream.Splice(sbufNewPart);
}

void TextOutputStreamFilter::Flush(bool bHard){
	y_vStream.Flush(bHard);
}

}
