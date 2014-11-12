// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"
#include "Exception.hpp"
#include <iterator>
using namespace MCF;

namespace {

// https://en.wikipedia.org/wiki/UTF-8
// https://en.wikipedia.org/wiki/UTF-16
template<class InputCharT>
unsigned long LoadUtf8(const InputCharT *&pchRead, const InputCharT *pchEnd){
	ASSERT(pchRead != pchEnd);

	unsigned long ulPoint = (std::uint8_t)*pchRead;
	++pchRead;
	if((ulPoint & 0x80u) == 0){
		// ASCII 字符。
		return ulPoint;
	}
	// 这个值是该码点的总字节数减一。
	const auto uAdditional = CountLeadingZeroes(~(ulPoint << (BITS_OF(ulPoint) - 7)) | 1);
	// UTF-8 理论上最长可以编码 6 个字符，但是标准化以后只能使用 4 个。
	if(uAdditional - 1 > 3){ // 0 - 3
		MCF_THROW(ERROR_INVALID_DATA, L"无效的 UTF-8 前导字符。");
	}
	if((std::size_t)(pchEnd - pchRead) < uAdditional){
		MCF_THROW(ERROR_HANDLE_EOF, L"UTF-8 字符串被截断。");
	}

	const auto pchPointEnd = pchRead + uAdditional;
	ulPoint &= (0x7Fu >> uAdditional);
	do {
		const unsigned long ulUnit = (std::uint8_t)*pchRead;
		if((ulUnit & 0xC0u) != 0x80u){
			// 编码错误。
			MCF_THROW(ERROR_INVALID_DATA, L"无效的 UTF-8 后续字符。");
		}
		ulPoint = (ulPoint << 6) | (ulUnit & 0x3Fu);
	} while(++pchRead != pchPointEnd);

	if(ulPoint > 0x10FFFFu){
		// 无效的 UTF-32 码点。
		MCF_THROW(ERROR_INVALID_DATA, L"UTF-32 码点的值超过范围。");
	}
	return ulPoint;
}
template<class OutputIteratorT>
void SaveUtf8(OutputIteratorT &itOutput, unsigned long ulPoint){
	// 这个值是该码点的总字节数减一。
	const auto uAdditional = (std::size_t)(29 - CountLeadingZeroes(ulPoint | 1)) / 5;
	if(uAdditional == 0){
		ASSERT(ulPoint <= 0x7Fu);
		*itOutput = (char)ulPoint;
		++itOutput;
	} else {
		ulPoint |= 0xFFFFFF80u << (uAdditional * 5);
		itOutput = (char)(ulPoint >> (uAdditional * 6));
		++itOutput;
		for(std::size_t i = uAdditional; i != 0; --i){
			itOutput = (char)(((ulPoint >> (i * 6 - 6)) & 0x3Fu) | 0x80u);
			++itOutput;
		}
	}
}

template<class OutputIteratorT, class InputCharT>
OutputIteratorT Utf32FromUtf8(OutputIteratorT itOutput, const InputCharT *pchBegin, const InputCharT *pchEnd){
	auto pchRead = pchBegin;
	while(pchRead != pchEnd){
		const auto ulPoint = LoadUtf8(pchRead, pchEnd);
		*itOutput = (char32_t)ulPoint;
		++itOutput;
	}
	return std::move(itOutput);
}
template<class OutputIteratorT>
OutputIteratorT Utf8FromUtf32(OutputIteratorT itOutput, const char32_t *pc32Begin, const char32_t *pc32End){
	auto pc32Read = pc32Begin;
	while(pc32Read != pc32End){
		unsigned long ulPoint = (std::uint32_t)*pc32Read;
		++pc32Read;
		if(ulPoint > 0x10FFFFu){
			// 无效的 UTF-32 码点。
			MCF_THROW(ERROR_INVALID_DATA, L"UTF-32 码点的值超过范围。");
		}
		SaveUtf8(itOutput, ulPoint);
	}
	return std::move(itOutput);
}

template<class OutputIteratorT, class InputCharT>
OutputIteratorT Utf32FromModifiedUtf8(OutputIteratorT itOutput, const InputCharT *pchBegin, const InputCharT *pchEnd){
	auto pchRead = pchBegin;
	while(pchRead != pchEnd){
		auto pchOldRead = pchRead;
		auto ulPoint = LoadUtf8(pchRead, pchEnd);
		// 检测前导代理。
		const auto ulLeading = ulPoint - 0xD800u;
		if(ulLeading > 0x7FFu){
			*itOutput = (char32_t)ulPoint;
			++itOutput;
			continue;
		}
		if(ulLeading > 0x3FFu){
			// 这是个后续代理。
			MCF_THROW(ERROR_INVALID_DATA, L"孤立的 UTF-16 后续代理。");
		}
		if(pchRead == pchEnd){
			MCF_THROW(ERROR_HANDLE_EOF, L"UTF-16 字符串被截断。");
		}
		ulPoint = LoadUtf8(pchRead, pchEnd);
		ulPoint -= 0xDC00u;
		if(ulPoint > 0x3FFu){
			// 后续代理无效。
			MCF_THROW(ERROR_INVALID_DATA, L"UTF-16 前导代理的后面不是后续代理。");
		}
		// 将代理对拼成一个码点。
		*itOutput = (char32_t)(((ulLeading << 10) | ulPoint) + 0x10000u);
		++itOutput;
	}
	return std::move(itOutput);
}
template<class OutputIteratorT>
OutputIteratorT ModifiedUtf8FromUtf32(OutputIteratorT itOutput, const char32_t *pc32Begin, const char32_t *pc32End){
	auto pc32Read = pc32Begin;
	while(pc32Read != pc32End){
		unsigned long ulPoint = (std::uint32_t)*pc32Read;
		++pc32Read;
		if(ulPoint == 0){
			*itOutput = (char)0xC0;
			++itOutput;
			*itOutput = (char)0x80;
			++itOutput;
			continue;
		}
		if(ulPoint > 0x10FFFFu){
			// 无效的 UTF-32 码点。
			MCF_THROW(ERROR_INVALID_DATA, L"UTF-32 码点的值超过范围。");
		}
		if(ulPoint <= 0xFFFFu){
			// 单个的 UTF-16 字符。
			SaveUtf8(itOutput, ulPoint);
			continue;
		}
		// 编码成代理对。
		ulPoint -= 0x10000u;
		SaveUtf8(itOutput, (ulPoint >> 10) | 0xD800u);
		SaveUtf8(itOutput, (ulPoint & 0x3FFu) | 0xDC00u);
	}
	return std::move(itOutput);
}

template<class OutputIteratorT, class InputCharT>
OutputIteratorT Utf32FromUtf16(OutputIteratorT itOutput, const InputCharT *pchBegin, const InputCharT *pchEnd){
	auto pchRead = pchBegin;
	while(pchRead != pchEnd){
		unsigned long ulPoint = (std::uint16_t)*pchRead;
		++pchRead;
		// 检测前导代理。
		const auto ulLeading = ulPoint - 0xD800u;
		if(ulLeading > 0x7FFu){
			*itOutput = (char32_t)ulPoint;
			++itOutput;
			continue;
		}
		if(ulLeading > 0x3FFu){
			// 这是个后续代理。
			MCF_THROW(ERROR_INVALID_DATA, L"孤立的 UTF-16 后续代理。");
		}
		if(pchRead == pchEnd){
			MCF_THROW(ERROR_HANDLE_EOF, L"UTF-16 字符串被截断。");
		}
		ulPoint = (std::uint16_t)*pchRead;
		++pchRead;
		ulPoint -= 0xDC00u;
		if(ulPoint > 0x3FFu){
			// 后续代理无效。
			MCF_THROW(ERROR_INVALID_DATA, L"UTF-16 前导代理的后面不是后续代理。");
		}
		// 将代理对拼成一个码点。
		*itOutput = (char32_t)(((ulLeading << 10) | ulPoint) + 0x10000u);
		++itOutput;
	}
	return std::move(itOutput);
}
template<class OutputIteratorT>
OutputIteratorT Utf16FromUtf32(OutputIteratorT itOutput, const char32_t *pc32Begin, const char32_t *pc32End){
	auto pc32Read = pc32Begin;
	while(pc32Read != pc32End){
		unsigned long ulPoint = (std::uint32_t)*pc32Read;
		++pc32Read;
		if(ulPoint > 0x10FFFFu){
			// 无效的 UTF-32 码点。
			MCF_THROW(ERROR_INVALID_DATA, L"UTF-32 码点的值超过范围。");
		}
		if(ulPoint <= 0xFFFFu){
			// 单个的 UTF-16 字符。
			*itOutput = (char32_t)ulPoint;
			++itOutput;
			continue;
		}
		// 编码成代理对。
		ulPoint -= 0x10000u;
		*itOutput = (char32_t)((ulPoint >> 10) | 0xD800u);
		++itOutput;
		*itOutput = (char32_t)((ulPoint & 0x3FFu) | 0xDC00u);
		++itOutput;
	}
	return std::move(itOutput);
}

}

namespace MCF {

template class String<StringTypes::NARROW>;
template class String<StringTypes::WIDE>;
template class String<StringTypes::UTF8>;
template class String<StringTypes::UTF16>;
template class String<StringTypes::UTF32>;
template class String<StringTypes::MOD_UTF8>;

// ANSI
template<>
UnifiedStringObserver NarrowString::Unify(UnifiedString &&usTempStorage, const NarrowStringObserver &nsoSrc){
	return {};
}
template<>
void NarrowString::Deunify(NarrowString &nsDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
}

// UTF-16
template<>
UnifiedStringObserver WideString::Unify(UnifiedString &&usTempStorage, const WideStringObserver &wsoSrc){
	usTempStorage.Reserve(wsoSrc.GetSize());
	Utf32FromUtf16(std::back_inserter(usTempStorage), wsoSrc.GetBegin(), wsoSrc.GetEnd());
	return usTempStorage;
}
template<>
void WideString::Deunify(WideString &wsDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	wsDst.ReserveMore(usoSrc.GetSize());
	Utf16FromUtf32(std::back_inserter(wsDst), usoSrc.GetBegin(), usoSrc.GetEnd());
}

// UTF-8
template<>
UnifiedStringObserver Utf8String::Unify(UnifiedString &&usTempStorage, const Utf8StringObserver &u8soSrc){
	usTempStorage.Reserve(u8soSrc.GetSize() * 3);
	Utf32FromUtf8(std::back_inserter(usTempStorage), u8soSrc.GetBegin(), u8soSrc.GetEnd());
	for(unsigned x : usTempStorage){
		std::printf("%04X ", x);
	}
	return usTempStorage;
}
template<>
void Utf8String::Deunify(Utf8String &u8sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	u8sDst.ReserveMore(usoSrc.GetSize() * 3);
	Utf8FromUtf32(std::back_inserter(u8sDst), usoSrc.GetBegin(), usoSrc.GetEnd());
}

// UTF-16
template<>
UnifiedStringObserver Utf16String::Unify(UnifiedString &&usTempStorage, const Utf16StringObserver &u16soSrc){
	usTempStorage.Reserve(u16soSrc.GetSize());
	Utf32FromUtf16(std::back_inserter(usTempStorage), u16soSrc.GetBegin(), u16soSrc.GetEnd());
	return usTempStorage;
}
template<>
void Utf16String::Deunify(Utf16String &u16sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	u16sDst.ReserveMore(usoSrc.GetSize());
	Utf16FromUtf32(std::back_inserter(u16sDst), usoSrc.GetBegin(), usoSrc.GetEnd());
}

// UTF-32
template<>
UnifiedStringObserver Utf32String::Unify(UnifiedString && /* usTempStorage */, const Utf32StringObserver &u32soSrc){
	return u32soSrc;
}
template<>
void Utf32String::Deunify(Utf32String &u32sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	u32sDst.Replace((std::ptrdiff_t)uPos, (std::ptrdiff_t)uPos, usoSrc);
}

// Modified UTF-8
template<>
UnifiedStringObserver ModUtf8String::Unify(UnifiedString &&usTempStorage, const ModUtf8StringObserver &mu8soSrc){
	usTempStorage.Reserve(mu8soSrc.GetSize() * 3);
	Utf32FromModifiedUtf8(std::back_inserter(usTempStorage), mu8soSrc.GetBegin(), mu8soSrc.GetEnd());
	return usTempStorage;
}
template<>
void ModUtf8String::Deunify(ModUtf8String &mu8sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	mu8sDst.ReserveMore(usoSrc.GetSize() * 3);
	ModifiedUtf8FromUtf32(std::back_inserter(mu8sDst), usoSrc.GetBegin(), usoSrc.GetEnd());
}

}
