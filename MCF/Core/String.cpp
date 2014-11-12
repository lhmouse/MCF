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
// https://en.wikipedia.org/wiki/CESU-8

template<typename CharT>
class StringSource {
private:
	const CharT *xm_pchRead;
	const CharT *const xm_pchEnd;

public:
	StringSource(const CharT *pchBegin, const CharT *pchEnd) noexcept
		: xm_pchRead(pchBegin), xm_pchEnd(pchEnd)
	{
	}

public:
	explicit operator bool() const noexcept {
		return xm_pchRead != xm_pchEnd;
	}
	unsigned long operator()(){
		if(xm_pchRead == xm_pchEnd){
			MCF_THROW(ERROR_HANDLE_EOF, L"遇到字符串结尾。");
		}
		return static_cast<std::make_unsigned_t<CharT>>(*(xm_pchRead++));
	}
};

template<class StringObserverT>
auto MakeStringSource(const StringObserverT &soRead) noexcept {
	return StringSource<typename StringObserverT::Char>(soRead.GetBegin(), soRead.GetEnd());
}

template<class PrevT>
class Utf8Decoder {
private:
	PrevT xm_vPrev;

public:
	explicit Utf8Decoder(PrevT vPrev)
		: xm_vPrev(std::move(vPrev))
	{
	}

public:
	explicit operator bool() const noexcept {
		return !!xm_vPrev;
	}
	unsigned long operator()(){
		auto ulPoint = xm_vPrev();
		if((ulPoint & 0x80u) != 0){
			// 这个值是该码点的总字节数。
			const auto uBytes = CountLeadingZeroes((std::uint8_t)(~ulPoint | 1));
			// UTF-8 理论上最长可以编码 6 个字符，但是标准化以后最多只能使用 4 个。
			if(uBytes - 2 > 2){ // 2, 3, 4
				MCF_THROW(ERROR_INVALID_DATA, L"无效的 UTF-8 前导字符。");
			}
			ulPoint &= (0xFFu >> uBytes);
			for(std::size_t i = 1; i < uBytes; ++i){
				const auto ulTemp = xm_vPrev();
				if((ulTemp & 0xC0u) != 0x80u){
					// 编码错误。
					MCF_THROW(ERROR_INVALID_DATA, L"无效的 UTF-8 后续字符。");
				}
				ulPoint = (ulPoint << 6) | (ulTemp & 0x3Fu);
			}
			if(ulPoint > 0x10FFFFu){
				// 无效的 UTF-32 码点。
				MCF_THROW(ERROR_INVALID_DATA, L"UTF-32 码点的值超过范围。");
			}
		}
		return ulPoint;
	}
};

template<class PrevT>
auto MakeUtf8Decoder(PrevT vPrev){
	return Utf8Decoder<PrevT>(std::move(vPrev));
}

template<class PrevT>
class Utf8Encoder {
private:
	PrevT xm_vPrev;
	unsigned long xm_ulPending;

public:
	explicit Utf8Encoder(PrevT vPrev)
		: xm_vPrev(std::move(vPrev)), xm_ulPending(0)
	{
	}

public:
	explicit operator bool() const noexcept {
		return xm_ulPending || !!xm_vPrev;
	}
	unsigned long operator()(){
		if(xm_ulPending){
			const auto ulRet = xm_ulPending & 0xFFu;
			xm_ulPending >>= 8;
			return ulRet;
		}
		auto ulPoint = xm_vPrev();
		if(ulPoint > 0x10FFFFu){
			// 无效的 UTF-32 码点。
			MCF_THROW(ERROR_INVALID_DATA, L"UTF-32 码点的值超过范围。");
		}
		// 这个值是该码点的总字节数。
		const auto uBytes = (34u - CountLeadingZeroes((std::uint32_t)(ulPoint | 0x7F))) / 5u;
		if(uBytes > 1){
			for(std::size_t i = 1; i < uBytes; ++i){
				xm_ulPending <<= 8;
				xm_ulPending |= (ulPoint & 0x3F) | 0x80u;
				ulPoint >>= 6;
			}
			ulPoint |= -0x100ul >> uBytes;
		}
		return ulPoint;
	}
};

template<class PrevT>
auto MakeUtf8Encoder(PrevT vPrev){
	return Utf8Encoder<PrevT>(std::move(vPrev));
}

template<class PrevT>
class Utf16Decoder {
private:
	PrevT xm_vPrev;

public:
	explicit Utf16Decoder(PrevT vPrev)
		: xm_vPrev(std::move(vPrev))
	{
	}

public:
	explicit operator bool() const noexcept {
		return !!xm_vPrev;
	}
	unsigned long operator()(){
		auto ulPoint = xm_vPrev();
		// 检测前导代理。
		const auto ulLeading = ulPoint - 0xD800u;
		if(ulLeading <= 0x7FFu){
			if(ulLeading > 0x3FFu){
				// 这是个后续代理。
				MCF_THROW(ERROR_INVALID_DATA, L"孤立的 UTF-16 后续代理。");
			}
			ulPoint = xm_vPrev() - 0xDC00u;
			if(ulPoint > 0x3FFu){
				// 后续代理无效。
				MCF_THROW(ERROR_INVALID_DATA, L"UTF-16 前导代理的后面不是后续代理。");
			}
			// 将代理对拼成一个码点。
			ulPoint = ((ulLeading << 10) | ulPoint) + 0x10000u;
		}
		return ulPoint;
	}
};

template<class PrevT>
auto MakeUtf16Decoder(PrevT vPrev){
	return Utf16Decoder<PrevT>(std::move(vPrev));
}

template<class PrevT>
class Utf16Encoder {
private:
	PrevT xm_vPrev;
	unsigned long xm_ulPending;

public:
	explicit Utf16Encoder(PrevT vPrev)
		: xm_vPrev(std::move(vPrev)), xm_ulPending(0)
	{
	}

public:
	explicit operator bool() const noexcept {
		return xm_ulPending || !!xm_vPrev;
	}
	unsigned long operator()(){
		if(xm_ulPending){
			const auto ulRet = xm_ulPending;
			xm_ulPending >>= 16;
			return ulRet;
		}
		auto ulPoint = xm_vPrev();
		if(ulPoint > 0x10FFFFu){
			// 无效的 UTF-32 码点。
			MCF_THROW(ERROR_INVALID_DATA, L"UTF-32 码点的值超过范围。");
		}
		if(ulPoint > 0xFFFFu){
			// 编码成代理对。
			ulPoint -= 0x10000u;
			xm_ulPending = (ulPoint & 0x3FFu) | 0xDC00u;
			ulPoint = (ulPoint >> 10) | 0xD800u;
		}
		return ulPoint;
	}
};

template<class PrevT>
auto MakeUtf16Encoder(PrevT vPrev){
	return Utf16Encoder<PrevT>(std::move(vPrev));
}

template<class StringT, class FilterT>
void Convert(StringT &strWrite, std::size_t uPos, FilterT vFilter){
	if(uPos == strWrite.GetSize()){
		while(vFilter){
			strWrite.Push(vFilter());
		}
	} else {
		typename StringT::Char achTemp[256];
		auto pchWrite = std::begin(achTemp);
		while(vFilter){
			*pchWrite = vFilter();
			if(++pchWrite == std::end(achTemp)){
				strWrite.Replace((std::ptrdiff_t)uPos, (std::ptrdiff_t)uPos, std::begin(achTemp), pchWrite);
				uPos += COUNT_OF(achTemp);
				pchWrite = std::begin(achTemp);
			}
		}
		if(pchWrite != std::begin(achTemp)){
			strWrite.Replace((std::ptrdiff_t)uPos, (std::ptrdiff_t)uPos, std::begin(achTemp), pchWrite);
		}
	}
}

}

namespace MCF {

template class String<StringTypes::NARROW>;
template class String<StringTypes::WIDE>;
template class String<StringTypes::UTF8>;
template class String<StringTypes::UTF16>;
template class String<StringTypes::UTF32>;
template class String<StringTypes::CESU8>;

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
	Convert(usTempStorage, 0,
		MakeUtf16Decoder(MakeStringSource(wsoSrc)));
	return usTempStorage;
}
template<>
void WideString::Deunify(WideString &wsDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	wsDst.ReserveMore(usoSrc.GetSize());
	Convert(wsDst, uPos,
		MakeUtf16Encoder(MakeStringSource(usoSrc)));
}

// UTF-8
template<>
UnifiedStringObserver Utf8String::Unify(UnifiedString &&usTempStorage, const Utf8StringObserver &u8soSrc){
	usTempStorage.Reserve(u8soSrc.GetSize());
	Convert(usTempStorage, 0,
		MakeUtf8Decoder(MakeStringSource(u8soSrc)));
	return usTempStorage;
}
template<>
void Utf8String::Deunify(Utf8String &u8sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	u8sDst.ReserveMore(usoSrc.GetSize() * 2);
	Convert(u8sDst, uPos,
		MakeUtf8Encoder(MakeStringSource(usoSrc)));
}

// UTF-16
template<>
UnifiedStringObserver Utf16String::Unify(UnifiedString &&usTempStorage, const Utf16StringObserver &u16soSrc){
	usTempStorage.Reserve(u16soSrc.GetSize());
	Convert(usTempStorage, 0,
		MakeUtf16Decoder(MakeStringSource(u16soSrc)));
	return usTempStorage;
}
template<>
void Utf16String::Deunify(Utf16String &u16sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	u16sDst.ReserveMore(usoSrc.GetSize());
	Convert(u16sDst, uPos,
		MakeUtf16Encoder(MakeStringSource(usoSrc)));
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

// CESU-8
template<>
UnifiedStringObserver Cesu8String::Unify(UnifiedString &&usTempStorage, const Cesu8StringObserver &cu8soSrc){
	usTempStorage.Reserve(cu8soSrc.GetSize());
	Convert(usTempStorage, 0,
		MakeUtf16Decoder(MakeUtf8Decoder(MakeStringSource(cu8soSrc))));
	return usTempStorage;
}
template<>
void Cesu8String::Deunify(Cesu8String &cu8sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	cu8sDst.ReserveMore(usoSrc.GetSize() * 2);
	Convert(cu8sDst, uPos,
		MakeUtf8Encoder(MakeUtf16Encoder(MakeStringSource(usoSrc))));
}

}
