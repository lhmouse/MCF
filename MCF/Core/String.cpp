// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"
#include "Exception.hpp"

namespace MCF {

namespace {
	// https://en.wikipedia.org/wiki/UTF-8
	// https://en.wikipedia.org/wiki/UTF-16
	// https://en.wikipedia.org/wiki/CESU-8

	template<typename CharT>
	class StringSource {
	private:
		const CharT *x_pchRead;
		const CharT *const x_pchEnd;

	public:
		StringSource(const CharT *pchBegin, const CharT *pchEnd) noexcept
			: x_pchRead(pchBegin), x_pchEnd(pchEnd)
		{
		}

	public:
		explicit operator bool() const noexcept {
			return x_pchRead != x_pchEnd;
		}
		char32_t operator()(){
			if(x_pchRead == x_pchEnd){
				DEBUG_THROW(Exception, "String is truncated", ERROR_HANDLE_EOF);
			}
			return static_cast<std::make_unsigned_t<CharT>>(*(x_pchRead++));
		}
	};

	template<class StringObserverT>
	auto MakeStringSource(const StringObserverT &soRead) noexcept {
		return StringSource<typename StringObserverT::CharType>(soRead.GetBegin(), soRead.GetEnd());
	}

	template<class PrevT, bool IS_CESU8_T>
	class Utf8Decoder {
	private:
		PrevT x_vPrev;

	public:
		explicit Utf8Decoder(PrevT vPrev)
			: x_vPrev(std::move(vPrev))
		{
		}

	public:
		explicit operator bool() const noexcept {
			return !!x_vPrev;
		}
		char32_t operator()(){
			auto c32Point = x_vPrev();
			if((c32Point & 0x80u) != 0){
				// 这个值是该码点的总字节数。
				const auto uBytes = CountLeadingZeroes((std::uint8_t)(~c32Point | 1));
				// UTF-8 理论上最长可以编码 6 个字符，但是标准化以后最多只能使用 4 个。
				if(uBytes - 2 > 2){ // 2, 3, 4
					DEBUG_THROW(Exception, "Invalid UTF-8 leading byte", ERROR_INVALID_DATA);
				}
				c32Point &= (0xFFu >> uBytes);
				for(std::size_t i = 1; i < uBytes; ++i){
					const auto c32Temp = x_vPrev();
					if((c32Temp & 0xC0u) != 0x80u){
						DEBUG_THROW(Exception, "Invalid UTF-8 non-leading byte", ERROR_INVALID_DATA);
					}
					c32Point = (c32Point << 6) | (c32Temp & 0x3Fu);
				}
				if(c32Point > 0x10FFFFu){
					DEBUG_THROW(Exception, "Invalid UTF-32 code point value", ERROR_INVALID_DATA);
				}
				if(!IS_CESU8_T && (c32Point - 0xD800 < 0x800)){
					DEBUG_THROW(Exception, "UTF-32 code point is reserved for UTF-16", ERROR_INVALID_DATA);
				}
			}
			return c32Point;
		}
	};

	template<class PrevT>
	auto MakeUtf8Decoder(PrevT vPrev){
		return Utf8Decoder<PrevT, false>(std::move(vPrev));
	}
	template<class PrevT>
	auto MakeCesu8Decoder(PrevT vPrev){
		return Utf8Decoder<PrevT, true>(std::move(vPrev));
	}

	template<class PrevT>
	class Utf8Encoder {
	private:
		PrevT x_vPrev;
		char32_t x_c32Pending;

	public:
		explicit Utf8Encoder(PrevT vPrev)
			: x_vPrev(std::move(vPrev)), x_c32Pending(0)
		{
		}

	public:
		explicit operator bool() const noexcept {
			return x_c32Pending || !!x_vPrev;
		}
		char32_t operator()(){
			if(x_c32Pending){
				const auto c32Ret = x_c32Pending & 0xFFu;
				x_c32Pending >>= 8;
				return c32Ret;
			}
			auto c32Point = x_vPrev();
			if(c32Point > 0x10FFFFu){
				DEBUG_THROW(Exception, "Invalid UTF-32 code point value", ERROR_INVALID_DATA);
			}
			// 这个值是该码点的总字节数。
			const auto uBytes = (34u - CountLeadingZeroes((std::uint32_t)(c32Point | 0x7F))) / 5u;
			if(uBytes > 1){
				for(std::size_t i = 1; i < uBytes; ++i){
					x_c32Pending <<= 8;
					x_c32Pending |= (c32Point & 0x3F) | 0x80u;
					c32Point >>= 6;
				}
				c32Point |= -0x100u >> uBytes;
			}
			return c32Point;
		}
	};

	template<class PrevT>
	auto MakeUtf8Encoder(PrevT vPrev){
		return Utf8Encoder<PrevT>(std::move(vPrev));
	}

	template<class PrevT>
	class Utf16Decoder {
	private:
		PrevT x_vPrev;

	public:
		explicit Utf16Decoder(PrevT vPrev)
			: x_vPrev(std::move(vPrev))
		{
		}

	public:
		explicit operator bool() const noexcept {
			return !!x_vPrev;
		}
		char32_t operator()(){
			auto c32Point = x_vPrev();
			// 检测前导代理。
			const auto c32Leading = c32Point - 0xD800u;
			if(c32Leading <= 0x7FFu){
				if(c32Leading > 0x3FFu){
					DEBUG_THROW(Exception, "Isolated UTF-16 trailing surrogate", ERROR_INVALID_DATA);
				}
				c32Point = x_vPrev() - 0xDC00u;
				if(c32Point > 0x3FFu){
					// 后续代理无效。
					DEBUG_THROW(Exception, "Leading surrogate followed by non-trailing-surrogate", ERROR_INVALID_DATA);
				}
				// 将代理对拼成一个码点。
				c32Point = ((c32Leading << 10) | c32Point) + 0x10000u;
			}
			return c32Point;
		}
	};

	template<class PrevT>
	auto MakeUtf16Decoder(PrevT vPrev){
		return Utf16Decoder<PrevT>(std::move(vPrev));
	}

	template<class PrevT>
	class Utf16Encoder {
	private:
		PrevT x_vPrev;
		char32_t x_c32Pending;

	public:
		explicit Utf16Encoder(PrevT vPrev)
			: x_vPrev(std::move(vPrev)), x_c32Pending(0)
		{
		}

	public:
		explicit operator bool() const noexcept {
			return x_c32Pending || !!x_vPrev;
		}
		char32_t operator()(){
			if(x_c32Pending){
				const auto c32Ret = x_c32Pending;
				x_c32Pending >>= 16;
				return c32Ret;
			}
			auto c32Point = x_vPrev();
			if(c32Point > 0x10FFFFu){
				DEBUG_THROW(Exception, "Invalid UTF-32 code point value", ERROR_INVALID_DATA);
			}
			if(c32Point > 0xFFFFu){
				// 编码成代理对。
				c32Point -= 0x10000u;
				x_c32Pending = (c32Point & 0x3FFu) | 0xDC00u;
				c32Point = (c32Point >> 10) | 0xD800u;
			}
			return c32Point;
		}
	};

	template<class PrevT>
	auto MakeUtf16Encoder(PrevT vPrev){
		return Utf16Encoder<PrevT>(std::move(vPrev));
	}

	template<class StringT, class FilterT>
	void Convert(StringT &strWrite, std::size_t uPos, FilterT vFilter){
		typename StringT::CharType achTemp[256];
		auto pchWrite = std::begin(achTemp);

		if(uPos == strWrite.GetSize()){
			while(vFilter){
				*pchWrite = vFilter();
				if(++pchWrite == std::end(achTemp)){
					strWrite.Append(std::begin(achTemp), pchWrite);
					pchWrite = std::begin(achTemp);
				}
			}
			if(pchWrite != std::begin(achTemp)){
				strWrite.Append(std::begin(achTemp), pchWrite);
			}
		} else {
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

template class String<StringType::NARROW>;
template class String<StringType::WIDE>;
template class String<StringType::UTF8>;
template class String<StringType::UTF16>;
template class String<StringType::UTF32>;
template class String<StringType::CESU8>;
template class String<StringType::ANSI>;

// UTF-8
template<>
UnifiedStringObserver NarrowString::Unify(UnifiedString &usTempStorage, const NarrowStringObserver &nsoSrc){
	usTempStorage.Reserve(nsoSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf8Decoder(MakeStringSource(nsoSrc)));
	return usTempStorage;
}
template<>
void NarrowString::Deunify(NarrowString &nsDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	nsDst.ReserveMore(usoSrc.GetSize() * 2);
	Convert(nsDst, uPos, MakeUtf8Encoder(MakeStringSource(usoSrc)));
}

// UTF-16
template<>
UnifiedStringObserver WideString::Unify(UnifiedString &usTempStorage, const WideStringObserver &wsoSrc){
	usTempStorage.Reserve(wsoSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf16Decoder(MakeStringSource(wsoSrc)));
	return usTempStorage;
}
template<>
void WideString::Deunify(WideString &wsDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	wsDst.ReserveMore(usoSrc.GetSize());
	Convert(wsDst, uPos, MakeUtf16Encoder(MakeStringSource(usoSrc)));
}

// UTF-8
template<>
UnifiedStringObserver Utf8String::Unify(UnifiedString &usTempStorage, const Utf8StringObserver &u8soSrc){
	usTempStorage.Reserve(u8soSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf8Decoder(MakeStringSource(u8soSrc)));
	return usTempStorage;
}
template<>
void Utf8String::Deunify(Utf8String &u8sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	u8sDst.ReserveMore(usoSrc.GetSize() * 3);
	Convert(u8sDst, uPos, MakeUtf8Encoder(MakeStringSource(usoSrc)));
}

// UTF-16
template<>
UnifiedStringObserver Utf16String::Unify(UnifiedString &usTempStorage, const Utf16StringObserver &u16soSrc){
	usTempStorage.Reserve(u16soSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf16Decoder(MakeStringSource(u16soSrc)));
	return usTempStorage;
}
template<>
void Utf16String::Deunify(Utf16String &u16sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	u16sDst.ReserveMore(usoSrc.GetSize());
	Convert(u16sDst, uPos, MakeUtf16Encoder(MakeStringSource(usoSrc)));
}

// UTF-32
template<>
UnifiedStringObserver Utf32String::Unify(UnifiedString & /* usTempStorage */, const Utf32StringObserver &u32soSrc){
	return u32soSrc;
}
template<>
void Utf32String::Deunify(Utf32String &u32sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	u32sDst.Replace((std::ptrdiff_t)uPos, (std::ptrdiff_t)uPos, usoSrc);
}

// CESU-8
template<>
UnifiedStringObserver Cesu8String::Unify(UnifiedString &usTempStorage, const Cesu8StringObserver &cu8soSrc){
	usTempStorage.Reserve(cu8soSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf16Decoder(MakeCesu8Decoder(MakeStringSource(cu8soSrc))));
	return usTempStorage;
}
template<>
void Cesu8String::Deunify(Cesu8String &cu8sDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	cu8sDst.ReserveMore(usoSrc.GetSize() * 3);
	Convert(cu8sDst, uPos, MakeUtf8Encoder(MakeUtf16Encoder(MakeStringSource(usoSrc))));
}

// ANSI
template<>
UnifiedStringObserver AnsiString::Unify(UnifiedString &usTempStorage, const AnsiStringObserver &asoSrc){
	if(!asoSrc.IsEmpty()){
		WideString wsTemp;
		wsTemp.Resize(asoSrc.GetSize());
		const unsigned uCount = (unsigned)::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
			asoSrc.GetBegin(), (int)asoSrc.GetSize(), wsTemp.GetData(), (int)wsTemp.GetSize());
		if(uCount == 0){
			DEBUG_THROW(SystemError, "MultiByteToWideChar");
		}
		usTempStorage.Reserve(uCount);
		Convert(usTempStorage, 0, MakeUtf16Decoder(MakeStringSource(WideStringObserver(wsTemp.GetData(), uCount))));
	}
	return usTempStorage;
}
template<>
void AnsiString::Deunify(AnsiString &ansDst, std::size_t uPos, const UnifiedStringObserver &usoSrc){
	if(!usoSrc.IsEmpty()){
		WideString wsTemp;
		wsTemp.Reserve(usoSrc.GetSize());
		Convert(wsTemp, 0, MakeUtf16Encoder(MakeStringSource(usoSrc)));

		AnsiString ansConverted;
		ansConverted.Resize(wsTemp.GetSize() * 2);
		const unsigned uCount = (unsigned)::WideCharToMultiByte(CP_ACP, 0,
			wsTemp.GetData(), (int)wsTemp.GetSize(), ansConverted.GetData(), (int)ansConverted.GetSize(), nullptr, nullptr);
		if(uCount == 0){
			DEBUG_THROW(SystemError, "WideCharToMultiByte");
		}
		ansDst.Replace((std::ptrdiff_t)uPos, (std::ptrdiff_t)uPos, ansConverted.GetData(), uCount);
	}
}

}
