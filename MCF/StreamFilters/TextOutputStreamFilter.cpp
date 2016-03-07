// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextOutputStreamFilter.hpp"

namespace MCF {

TextOutputStreamFilter::~TextOutputStreamFilter(){
	try {
		X_FlushEncodedBuffer();
	} catch(...){
	}
}

void TextOutputStreamFilter::X_FlushEncodedBuffer(){
	if(x_vecEncoded.IsEmpty()){
		return;
	}

	y_vStream.Put(x_vecEncoded.GetData(), x_vecEncoded.GetSize());
	x_vecEncoded.Clear();
}

void TextOutputStreamFilter::Put(unsigned char byData){
	bool bShouldFlush = false;

	x_vecEncoded.ReserveMore(2);

	if(byData != '\n'){
		x_vecEncoded.UncheckedPush(static_cast<char>(byData));
	} else {
		x_vecEncoded.UncheckedPush('\r');
		x_vecEncoded.UncheckedPush('\n');
		bShouldFlush = true;
	}

	if(bShouldFlush){
		X_FlushEncodedBuffer();
	}
}

void TextOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	bool bShouldFlush = false;

	const auto uSizeToReserve = uSize * 2;
	if(uSizeToReserve / 2 != uSize){
		throw std::bad_array_new_length();
	}
	x_vecEncoded.ReserveMore(uSizeToReserve);

	auto pchLineBegin = static_cast<const char *>(pData);
	const auto pchEnd = pchLineBegin + uSize;
	for(;;){
		auto pchLineEnd = static_cast<const char *>(std::memchr(pchLineBegin, '\n', static_cast<std::size_t>(pchEnd - pchLineBegin)));
		if(!pchLineEnd){
			x_vecEncoded.UncheckedAppend(pchLineBegin, pchEnd);
			break;
		}
		x_vecEncoded.UncheckedAppend(pchLineBegin, pchLineEnd);
		x_vecEncoded.UncheckedPush('\r');
		x_vecEncoded.UncheckedPush('\n');
		bShouldFlush = true;
		pchLineBegin = pchLineEnd + 1;
	}

	if(bShouldFlush){
		X_FlushEncodedBuffer();
	}
}

void TextOutputStreamFilter::Flush(bool bHard){
	X_FlushEncodedBuffer();

	y_vStream.Flush(bHard);
}

}
