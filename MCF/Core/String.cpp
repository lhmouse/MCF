// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"
#include "Exception.hpp"
#include <winternl.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS RtlMultiByteToUnicodeN(wchar_t *pwcBuffer, ULONG ulBufferSize, ULONG *pulBytesMax, const char *pchMultiByteString, ULONG ulMultiByteStringSize) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS RtlUnicodeToMultiByteN(char *pchBuffer, ULONG ulBufferSize, ULONG *pulBytesMax, const wchar_t *pwcUnicodeString, ULONG ulUnicodeStringSize) noexcept;

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
		std::uint32_t operator()(){
			if(x_pchRead == x_pchEnd){
				DEBUG_THROW(Exception, ERROR_HANDLE_EOF, "String is truncated"_rcs);
			}
			return static_cast<std::make_unsigned_t<CharT>>(*(x_pchRead++));
		}
	};

	template<class StringViewT>
	auto MakeStringSource(const StringViewT &svRead) noexcept {
		return StringSource<typename StringViewT::Char>(svRead.GetBegin(), svRead.GetEnd());
	}

	template<class PrevT, bool kIsCesu8T>
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
		std::uint32_t operator()(){
			auto u32Point = x_vPrev();
			if(u32Point & 0x80u){
				// 这个值是该码点的总字节数。
				const auto uBytes = CountLeadingZeroes((std::uint8_t)(~u32Point | 1));
				// UTF-8 理论上最长可以编码 6 个字符，但是标准化以后最多只能使用 4 个。
				if(uBytes - 2 > 2){ // 2, 3, 4
					DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-8 leading byte"_rcs);
				}
				u32Point &= 0xFFu >> uBytes;

				switch(uBytes){

#define UTF8_DECODER_UNROLLED	\
					{	\
						const auto u32Temp = x_vPrev();	\
						if((u32Temp & 0xC0u) != 0x80u){	\
							DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-8 non-leading byte"_rcs);	\
						}	\
						u32Point = (u32Point << 6) | (u32Temp & 0x3Fu);	\
					}

				default:
					UTF8_DECODER_UNROLLED
				case 3:
					UTF8_DECODER_UNROLLED
				case 2:
					UTF8_DECODER_UNROLLED
				}

				if(u32Point > 0x10FFFFu){
					DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-32 code point value"_rcs);
				}
				if(!kIsCesu8T && (u32Point - 0xD800u < 0x800u)){
					DEBUG_THROW(Exception, ERROR_INVALID_DATA, "UTF-32 code point is reserved for UTF-16"_rcs);
				}
			}
			return u32Point;
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
		std::uint32_t x_u32Pending;

	public:
		explicit Utf8Encoder(PrevT vPrev)
			: x_vPrev(std::move(vPrev)), x_u32Pending(0)
		{
		}

	public:
		explicit operator bool() const noexcept {
			return x_u32Pending || !!x_vPrev;
		}
		std::uint32_t operator()(){
			if(x_u32Pending){
				const auto u32Ret = x_u32Pending & 0xFFu;
				x_u32Pending >>= 8;
				return u32Ret;
			}

			auto u32Point = x_vPrev();
			if(u32Point > 0x10FFFFu){
				DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-32 code point value"_rcs);
			}
			// 这个值是该码点的总字节数。
			const auto uBytes = (34u - CountLeadingZeroes((std::uint32_t)(u32Point | 0x7Fu))) / 5u;
			if(uBytes > 1){
				switch(uBytes){

#define UTF8_ENCODER_UNROLLED	\
					{	\
						x_u32Pending <<= 8;	\
						x_u32Pending |= (u32Point & 0x3F) | 0x80u;	\
						u32Point >>= 6;	\
					}

				default:
					UTF8_ENCODER_UNROLLED
				case 3:
					UTF8_ENCODER_UNROLLED
				case 2:
					UTF8_ENCODER_UNROLLED
				}

				u32Point |= ~0xFFu >> uBytes;
			}
			return u32Point;
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
		std::uint32_t operator()(){
			auto u32Point = x_vPrev();
			// 检测前导代理。
			const auto u32Leading = u32Point - 0xD800u;
			if(u32Leading <= 0x7FFu){
				if(u32Leading > 0x3FFu){
					DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Isolated UTF-16 trailing surrogate"_rcs);
				}
				u32Point = x_vPrev() - 0xDC00u;
				if(u32Point > 0x3FFu){
					// 后续代理无效。
					DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Leading surrogate followed by non-trailing-surrogate"_rcs);
				}
				// 将代理对拼成一个码点。
				u32Point = ((u32Leading << 10) | u32Point) + 0x10000u;
			}
			return u32Point;
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
		std::uint32_t x_u32Pending;

	public:
		explicit Utf16Encoder(PrevT vPrev)
			: x_vPrev(std::move(vPrev)), x_u32Pending(0)
		{
		}

	public:
		explicit operator bool() const noexcept {
			return x_u32Pending || !!x_vPrev;
		}
		std::uint32_t operator()(){
			if(x_u32Pending){
				const auto u32Ret = x_u32Pending;
				x_u32Pending >>= 16;
				return u32Ret;
			}

			auto u32Point = x_vPrev();
			if(u32Point > 0x10FFFFu){
				DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-32 code point value"_rcs);
			}
			if(u32Point > 0xFFFFu){
				// 编码成代理对。
				u32Point -= 0x10000u;
				x_u32Pending = (u32Point & 0x3FFu) | 0xDC00u;
				u32Point = (u32Point >> 10) | 0xD800u;
			}
			return u32Point;
		}
	};

	template<class PrevT>
	auto MakeUtf16Encoder(PrevT vPrev){
		return Utf16Encoder<PrevT>(std::move(vPrev));
	}

	template<class StringT, class FilterT>
	__attribute__((__flatten__))
	void Convert(StringT &strWrite, FilterT vFilter){
		typename StringT::Char achTemp[256];
		auto pchWrite = achTemp;

		while(vFilter){
			*pchWrite = vFilter();
			if(++pchWrite == std::end(achTemp)){
				strWrite.Append(achTemp, pchWrite);
				pchWrite = achTemp;
			}
		}
		if(pchWrite != achTemp){
			strWrite.Append(achTemp, pchWrite);
		}
	}
}

template class String<StringType::kNarrow>;
template class String<StringType::kWide>;
template class String<StringType::kUtf8>;
template class String<StringType::kUtf16>;
template class String<StringType::kUtf32>;
template class String<StringType::kCesu8>;
template class String<StringType::kAnsi>;

static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t does not have the same size with char16_t.");
static_assert(alignof(wchar_t) == alignof(char16_t), "wchar_t does not have the same alignment with char16_t.");

// UTF-8
template<>
__attribute__((__flatten__))
StringView<StringType::kUtf16> NarrowString::Unify(String<StringType::kUtf16> &u16sTemp, const NarrowStringView &vSrc){
	ASSERT(u16sTemp.IsEmpty());

	u16sTemp.Reserve(vSrc.GetSize());
	Convert(u16sTemp, MakeUtf16Encoder(MakeUtf8Decoder(MakeStringSource(vSrc))));
	return u16sTemp;
}
template<>
__attribute__((__flatten__))
void NarrowString::Deunify(NarrowString &strDst, const StringView<StringType::kUtf16> &u16svSrc){
	strDst.ReserveMore(u16svSrc.GetSize() * 3);
	Convert(strDst, MakeUtf8Encoder(MakeUtf16Decoder(MakeStringSource(u16svSrc))));
}

template<>
__attribute__((__flatten__))
StringView<StringType::kUtf32> NarrowString::Unify(String<StringType::kUtf32> &u32sTemp, const NarrowStringView &vSrc){
	ASSERT(u32sTemp.IsEmpty());

	u32sTemp.Reserve(vSrc.GetSize());
	Convert(u32sTemp, MakeUtf8Decoder(MakeStringSource(vSrc)));
	return u32sTemp;
}
template<>
__attribute__((__flatten__))
void NarrowString::Deunify(NarrowString &strDst, const StringView<StringType::kUtf32> &u32svSrc){
	strDst.ReserveMore(u32svSrc.GetSize() * 2);
	Convert(strDst, MakeUtf8Encoder(MakeStringSource(u32svSrc)));
}

// UTF-16
template<>
__attribute__((__flatten__))
StringView<StringType::kUtf16> WideString::Unify(String<StringType::kUtf16> & /* u16sTemp */, const WideStringView &vSrc){
	return StringView<StringType::kUtf16>(reinterpret_cast<const char16_t *>(vSrc.GetBegin()), vSrc.GetSize());
}
template<>
__attribute__((__flatten__))
void WideString::Deunify(WideString &strDst, const StringView<StringType::kUtf16> &u16svSrc){
	strDst.Append(reinterpret_cast<const wchar_t *>(u16svSrc.GetBegin()), u16svSrc.GetSize());
}

template<>
__attribute__((__flatten__))
StringView<StringType::kUtf32> WideString::Unify(String<StringType::kUtf32> &u32sTemp, const WideStringView &vSrc){
	ASSERT(u32sTemp.IsEmpty());

	u32sTemp.Reserve(vSrc.GetSize());
	Convert(u32sTemp, MakeUtf16Decoder(MakeStringSource(vSrc)));
	return u32sTemp;
}
template<>
__attribute__((__flatten__))
void WideString::Deunify(WideString &strDst, const StringView<StringType::kUtf32> &u32svSrc){
	strDst.ReserveMore(u32svSrc.GetSize());
	Convert(strDst, MakeUtf16Encoder(MakeStringSource(u32svSrc)));
}

// UTF-8
template<>
__attribute__((__flatten__))
StringView<StringType::kUtf16> Utf8String::Unify(String<StringType::kUtf16> &u16sTemp, const Utf8StringView &vSrc){
	ASSERT(u16sTemp.IsEmpty());

	u16sTemp.Reserve(vSrc.GetSize());
	Convert(u16sTemp, MakeUtf16Encoder(MakeUtf8Decoder(MakeStringSource(vSrc))));
	return u16sTemp;
}
template<>
__attribute__((__flatten__))
void Utf8String::Deunify(Utf8String &strDst, const StringView<StringType::kUtf16> &u16svSrc){
	strDst.ReserveMore(u16svSrc.GetSize() * 3);
	Convert(strDst, MakeUtf8Encoder(MakeUtf16Decoder(MakeStringSource(u16svSrc))));
}

template<>
__attribute__((__flatten__))
StringView<StringType::kUtf32> Utf8String::Unify(String<StringType::kUtf32> &u32sTemp, const Utf8StringView &vSrc){
	ASSERT(u32sTemp.IsEmpty());

	u32sTemp.Reserve(vSrc.GetSize());
	Convert(u32sTemp, MakeUtf8Decoder(MakeStringSource(vSrc)));
	return u32sTemp;
}
template<>
__attribute__((__flatten__))
void Utf8String::Deunify(Utf8String &strDst, const StringView<StringType::kUtf32> &u32svSrc){
	strDst.ReserveMore(u32svSrc.GetSize() * 2);
	Convert(strDst, MakeUtf8Encoder(MakeStringSource(u32svSrc)));
}

// UTF-16
template<>
__attribute__((__flatten__))
StringView<StringType::kUtf16> Utf16String::Unify(String<StringType::kUtf16> & /* u16sTemp */, const Utf16StringView &vSrc){
	return StringView<StringType::kUtf16>(vSrc.GetBegin(), vSrc.GetSize());
}
template<>
__attribute__((__flatten__))
void Utf16String::Deunify(Utf16String &strDst, const StringView<StringType::kUtf16> &u16svSrc){
	strDst.Append(u16svSrc.GetBegin(), u16svSrc.GetSize());
}

template<>
__attribute__((__flatten__))
StringView<StringType::kUtf32> Utf16String::Unify(String<StringType::kUtf32> &u32sTemp, const Utf16StringView &vSrc){
	ASSERT(u32sTemp.IsEmpty());

	u32sTemp.Reserve(vSrc.GetSize());
	Convert(u32sTemp, MakeUtf16Decoder(MakeStringSource(vSrc)));
	return u32sTemp;
}
template<>
__attribute__((__flatten__))
void Utf16String::Deunify(Utf16String &strDst, const StringView<StringType::kUtf32> &u32svSrc){
	strDst.ReserveMore(u32svSrc.GetSize());
	Convert(strDst, MakeUtf16Encoder(MakeStringSource(u32svSrc)));
}

// UTF-32
template<>
__attribute__((__flatten__))
StringView<StringType::kUtf16> Utf32String::Unify(String<StringType::kUtf16> &u16sTemp, const Utf32StringView &vSrc){
	ASSERT(u16sTemp.IsEmpty());

	u16sTemp.Reserve(vSrc.GetSize());
	Convert(u16sTemp, MakeUtf16Encoder(MakeStringSource(vSrc)));
	return u16sTemp;
}
template<>
__attribute__((__flatten__))
void Utf32String::Deunify(Utf32String &strDst, const StringView<StringType::kUtf16> &u16svSrc){
	strDst.ReserveMore(u16svSrc.GetSize());
	Convert(strDst, MakeUtf16Decoder(MakeStringSource(u16svSrc)));
}

template<>
__attribute__((__flatten__))
StringView<StringType::kUtf32> Utf32String::Unify(String<StringType::kUtf32> & /* u32sTemp */, const Utf32StringView &vSrc){
	return StringView<StringType::kUtf32>(vSrc.GetBegin(), vSrc.GetSize());
}
template<>
__attribute__((__flatten__))
void Utf32String::Deunify(Utf32String &strDst, const StringView<StringType::kUtf32> &u32svSrc){
	strDst.Append(u32svSrc.GetBegin(), u32svSrc.GetSize());
}

// CESU-8
template<>
__attribute__((__flatten__))
StringView<StringType::kUtf16> Cesu8String::Unify(String<StringType::kUtf16> &u16sTemp, const Cesu8StringView &vSrc){
	ASSERT(u16sTemp.IsEmpty());

	u16sTemp.Reserve(vSrc.GetSize());
	Convert(u16sTemp, MakeCesu8Decoder(MakeStringSource(vSrc)));
	return u16sTemp;
}
template<>
__attribute__((__flatten__))
void Cesu8String::Deunify(Cesu8String &strDst, const StringView<StringType::kUtf16> &u16svSrc){
	strDst.ReserveMore(u16svSrc.GetSize() * 3);
	Convert(strDst, MakeUtf8Encoder(MakeStringSource(u16svSrc)));
}

template<>
__attribute__((__flatten__))
StringView<StringType::kUtf32> Cesu8String::Unify(String<StringType::kUtf32> &u32sTemp, const Cesu8StringView &vSrc){
	ASSERT(u32sTemp.IsEmpty());

	u32sTemp.Reserve(vSrc.GetSize());
	Convert(u32sTemp, MakeUtf16Decoder(MakeCesu8Decoder(MakeStringSource(vSrc))));
	return u32sTemp;
}
template<>
__attribute__((__flatten__))
void Cesu8String::Deunify(Cesu8String &strDst, const StringView<StringType::kUtf32> &u32svSrc){
	strDst.ReserveMore(u32svSrc.GetSize() * 2);
	Convert(strDst, MakeUtf8Encoder(MakeUtf16Encoder(MakeStringSource(u32svSrc))));
}

// ANSI
template<>
__attribute__((__flatten__))
StringView<StringType::kUtf16> AnsiString::Unify(String<StringType::kUtf16> &u16sTemp, const AnsiStringView &vSrc){
	ASSERT(u16sTemp.IsEmpty());

	const auto uInputSize = vSrc.GetSize();
	if(uInputSize > ULONG_MAX){
		DEBUG_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, "The input ANSI string is too long"_rcs);
	}
	const auto uOutputSizeMax = uInputSize * sizeof(wchar_t);
	if((uOutputSizeMax > ULONG_MAX) || (uOutputSizeMax / sizeof(wchar_t) != uInputSize)){
		DEBUG_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, "The output Unicode string requires more memory than ULONG_MAX bytes"_rcs);
	}
	u16sTemp.Resize(uOutputSizeMax / sizeof(wchar_t));
	ULONG ulConvertedSize;
	const auto lStatus = ::RtlMultiByteToUnicodeN(reinterpret_cast<wchar_t *>(u16sTemp.GetStr()), uOutputSizeMax, &ulConvertedSize, vSrc.GetBegin(), uInputSize);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "RtlMultiByteToUnicodeN"_rcs);
	}
	u16sTemp.Resize(ulConvertedSize / sizeof(wchar_t));
	return u16sTemp;
}
template<>
__attribute__((__flatten__))
void AnsiString::Deunify(AnsiString &strDst, const StringView<StringType::kUtf16> &u16svSrc){
	const auto uInputSize = u16svSrc.GetSize() * sizeof(wchar_t);
	if((uInputSize > ULONG_MAX) || (uInputSize / sizeof(wchar_t) != u16svSrc.GetSize())){
		DEBUG_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, "The input Unicode string is too long"_rcs);
	}
	const auto uOutputSizeMax = uInputSize * 2;
	if((uOutputSizeMax > ULONG_MAX) || (uOutputSizeMax / 2 != uInputSize)){
		DEBUG_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, "The output ANSI string requires more memory than ULONG_MAX bytes"_rcs);
	}
	const auto uOldSize = strDst.GetSize();
	const auto pchWrite = strDst.ResizeMore(uOutputSizeMax);
	ULONG ulConvertedSize;
	const auto lStatus = ::RtlUnicodeToMultiByteN(pchWrite, uOutputSizeMax, &ulConvertedSize, reinterpret_cast<const wchar_t *>(u16svSrc.GetBegin()), uInputSize);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "RtlUnicodeToMultiByteN"_rcs);
	}
	strDst.Resize(uOldSize + ulConvertedSize);
}

template<>
__attribute__((__flatten__))
StringView<StringType::kUtf32> AnsiString::Unify(String<StringType::kUtf32> &u32sTemp, const AnsiStringView &vSrc){
	ASSERT(u32sTemp.IsEmpty());

	Utf16String u16sTemp;
	const auto u16svResult = Unify(u16sTemp, vSrc);
	return Utf16String::Unify(u32sTemp, u16svResult);
}
template<>
__attribute__((__flatten__))
void AnsiString::Deunify(AnsiString &strDst, const StringView<StringType::kUtf32> &u32svSrc){
	Utf16String u16sTemp;
	Utf16String::Deunify(u16sTemp, u32svSrc);
	Deunify(strDst, u16sTemp);
}

}
