// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"
#include "Exception.hpp"
#include "../../MCFCRT/ext/expect.h"
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
		__attribute__((__always_inline__))
		explicit operator bool() const noexcept {
			return x_pchRead != x_pchEnd;
		}
		__attribute__((__always_inline__))
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
		__attribute__((__always_inline__))
		explicit operator bool() const noexcept {
			return !!x_vPrev;
		}
		__attribute__((__always_inline__))
		std::uint32_t operator()(){
			auto u32Point = x_vPrev();
			if(EXPECT_NOT((u32Point & 0x80u) != 0)){
				// 这个值是该码点的总字节数。
				const auto uBytes = CountLeadingZeroes((std::uint8_t)(~u32Point | 1));
				// UTF-8 理论上最长可以编码 6 个字符，但是标准化以后最多只能使用 4 个。
				if(EXPECT_NOT(uBytes - 2 > 2)){ // 2, 3, 4
					DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-8 leading byte"_rcs);
				}
				u32Point &= (0xFFu >> uBytes);

#define UTF8_DECODER_UNROLLED	\
				{	\
					const auto u32Temp = x_vPrev();	\
					if((u32Temp & 0xC0u) != 0x80u){	\
						DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-8 non-leading byte"_rcs);	\
					}	\
					u32Point = (u32Point << 6) | (u32Temp & 0x3Fu);	\
				}

				if(uBytes < 3){
					UTF8_DECODER_UNROLLED
				} else if(uBytes == 3){
					UTF8_DECODER_UNROLLED
					UTF8_DECODER_UNROLLED
				} else {
					UTF8_DECODER_UNROLLED
					UTF8_DECODER_UNROLLED
					UTF8_DECODER_UNROLLED
				}

				if(EXPECT_NOT(u32Point > 0x10FFFFu)){
					DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-32 code point value"_rcs);
				}
				if(EXPECT_NOT(!kIsCesu8T && (u32Point - 0xD800u < 0x800u))){
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
		__attribute__((__always_inline__))
		explicit operator bool() const noexcept {
			return x_u32Pending || !!x_vPrev;
		}
		__attribute__((__always_inline__))
		std::uint32_t operator()(){
			if(EXPECT(x_u32Pending != 0)){
				const auto u32Ret = x_u32Pending & 0xFFu;
				x_u32Pending >>= 8;
				return u32Ret;
			}

			auto u32Point = x_vPrev();
			if(EXPECT_NOT(u32Point > 0x10FFFFu)){
				DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-32 code point value"_rcs);
			}
			// 这个值是该码点的总字节数。
			const auto uBytes = (34u - CountLeadingZeroes((std::uint32_t)(u32Point | 0x7Fu))) / 5u;
			if(EXPECT_NOT(uBytes > 1)){

#define UTF8_ENCODER_UNROLLED	\
				{	\
					x_u32Pending <<= 8;	\
					x_u32Pending |= (u32Point & 0x3F) | 0x80u;	\
					u32Point >>= 6;	\
				}

				if(uBytes < 3){
					UTF8_ENCODER_UNROLLED
				} else if(uBytes == 3){
					UTF8_ENCODER_UNROLLED
					UTF8_ENCODER_UNROLLED
				} else {
					UTF8_ENCODER_UNROLLED
					UTF8_ENCODER_UNROLLED
					UTF8_ENCODER_UNROLLED
				}

				u32Point |= -0x100u >> uBytes;
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
		__attribute__((__always_inline__))
		explicit operator bool() const noexcept {
			return !!x_vPrev;
		}
		__attribute__((__always_inline__))
		std::uint32_t operator()(){
			auto u32Point = x_vPrev();
			// 检测前导代理。
			const auto u32Leading = u32Point - 0xD800u;
			if(EXPECT_NOT(u32Leading <= 0x7FFu)){
				if(EXPECT_NOT(u32Leading > 0x3FFu)){
					DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Isolated UTF-16 trailing surrogate"_rcs);
				}
				u32Point = x_vPrev() - 0xDC00u;
				if(EXPECT_NOT(u32Point > 0x3FFu)){
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
		__attribute__((__always_inline__))
		explicit operator bool() const noexcept {
			return x_u32Pending || !!x_vPrev;
		}
		__attribute__((__always_inline__))
		std::uint32_t operator()(){
			if(EXPECT(x_u32Pending != 0)){
				const auto u32Ret = x_u32Pending;
				x_u32Pending >>= 16;
				return u32Ret;
			}

			auto u32Point = x_vPrev();
			if(EXPECT_NOT(u32Point > 0x10FFFFu)){
				DEBUG_THROW(Exception, ERROR_INVALID_DATA, "Invalid UTF-32 code point value"_rcs);
			}
			if(EXPECT_NOT(u32Point > 0xFFFFu)){
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
	void Convert(StringT &strWrite, std::size_t uPos, FilterT vFilter){
		typename StringT::Char achTemp[1024];
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
					uPos += CountOf(achTemp);
					pchWrite = std::begin(achTemp);
				}
			}
			if(pchWrite != std::begin(achTemp)){
				strWrite.Replace((std::ptrdiff_t)uPos, (std::ptrdiff_t)uPos, std::begin(achTemp), pchWrite);
			}
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

// UTF-8
template<>
UnifiedStringView NarrowString::Unify(UnifiedString &usTempStorage, const NarrowStringView &nsvSrc){
	usTempStorage.Reserve(nsvSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf8Decoder(MakeStringSource(nsvSrc)));
	return usTempStorage;
}
template<>
void NarrowString::Deunify(NarrowString &nsDst, std::size_t uPos, const UnifiedStringView &usvSrc){
	nsDst.ReserveMore(usvSrc.GetSize() * 2);
	Convert(nsDst, uPos, MakeUtf8Encoder(MakeStringSource(usvSrc)));
}

// UTF-16
template<>
UnifiedStringView WideString::Unify(UnifiedString &usTempStorage, const WideStringView &wsvSrc){
	usTempStorage.Reserve(wsvSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf16Decoder(MakeStringSource(wsvSrc)));
	return usTempStorage;
}
template<>
void WideString::Deunify(WideString &wsDst, std::size_t uPos, const UnifiedStringView &usvSrc){
	wsDst.ReserveMore(usvSrc.GetSize());
	Convert(wsDst, uPos, MakeUtf16Encoder(MakeStringSource(usvSrc)));
}

// UTF-8
template<>
UnifiedStringView Utf8String::Unify(UnifiedString &usTempStorage, const Utf8StringView &u8svSrc){
	usTempStorage.Reserve(u8svSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf8Decoder(MakeStringSource(u8svSrc)));
	return usTempStorage;
}
template<>
void Utf8String::Deunify(Utf8String &u8sDst, std::size_t uPos, const UnifiedStringView &usvSrc){
	u8sDst.ReserveMore(usvSrc.GetSize() * 3);
	Convert(u8sDst, uPos, MakeUtf8Encoder(MakeStringSource(usvSrc)));
}

// UTF-16
template<>
UnifiedStringView Utf16String::Unify(UnifiedString &usTempStorage, const Utf16StringView &u16svSrc){
	usTempStorage.Reserve(u16svSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf16Decoder(MakeStringSource(u16svSrc)));
	return usTempStorage;
}
template<>
void Utf16String::Deunify(Utf16String &u16sDst, std::size_t uPos, const UnifiedStringView &usvSrc){
	u16sDst.ReserveMore(usvSrc.GetSize());
	Convert(u16sDst, uPos, MakeUtf16Encoder(MakeStringSource(usvSrc)));
}

// UTF-32
template<>
UnifiedStringView Utf32String::Unify(UnifiedString & /* usTempStorage */, const Utf32StringView &u32svSrc){
	return u32svSrc;
}
template<>
void Utf32String::Deunify(Utf32String &u32sDst, std::size_t uPos, const UnifiedStringView &usvSrc){
	u32sDst.Replace((std::ptrdiff_t)uPos, (std::ptrdiff_t)uPos, usvSrc);
}

// CESU-8
template<>
UnifiedStringView Cesu8String::Unify(UnifiedString &usTempStorage, const Cesu8StringView &cu8svSrc){
	usTempStorage.Reserve(cu8svSrc.GetSize());
	Convert(usTempStorage, 0, MakeUtf16Decoder(MakeCesu8Decoder(MakeStringSource(cu8svSrc))));
	return usTempStorage;
}
template<>
void Cesu8String::Deunify(Cesu8String &cu8sDst, std::size_t uPos, const UnifiedStringView &usvSrc){
	cu8sDst.ReserveMore(usvSrc.GetSize() * 3);
	Convert(cu8sDst, uPos, MakeUtf8Encoder(MakeUtf16Encoder(MakeStringSource(usvSrc))));
}

// ANSI
template<>
UnifiedStringView AnsiString::Unify(UnifiedString &usTempStorage, const AnsiStringView &asvSrc){
	if(asvSrc.IsEmpty()){
		return usTempStorage;
	}

	const auto uInputSize = asvSrc.GetSize();
	if(uInputSize > ULONG_MAX){
		DEBUG_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, "The input ANSI string is too long"_rcs);
	}
	const auto uOutputSizeMax = uInputSize * sizeof(wchar_t);
	if((uOutputSizeMax > ULONG_MAX) || (uOutputSizeMax / sizeof(wchar_t) != uInputSize)){
		DEBUG_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, "The output Unicode string requires more memory than ULONG_MAX bytes"_rcs);
	}
	WideString wsTemp;
	wsTemp.Resize(uOutputSizeMax / sizeof(wchar_t));
	ULONG ulConvertedSize;
	const auto lStatus = ::RtlMultiByteToUnicodeN(wsTemp.GetStr(), uOutputSizeMax, &ulConvertedSize, asvSrc.GetBegin(), uInputSize);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "RtlMultiByteToUnicodeN"_rcs);
	}
	wsTemp.Pop(wsTemp.GetSize() - ulConvertedSize / sizeof(wchar_t));

	usTempStorage.Reserve(ulConvertedSize / sizeof(wchar_t));
	Convert(usTempStorage, 0, MakeUtf16Decoder(MakeStringSource(wsTemp)));

	return usTempStorage;
}
template<>
void AnsiString::Deunify(AnsiString &ansDst, std::size_t uPos, const UnifiedStringView &usvSrc){
	if(usvSrc.IsEmpty()){
		return;
	}

	WideString wsTemp;
	wsTemp.Reserve(usvSrc.GetSize());
	Convert(wsTemp, 0, MakeUtf16Encoder(MakeStringSource(usvSrc)));

	const auto uInputSize = wsTemp.GetSize() * sizeof(wchar_t);
	if((uInputSize > ULONG_MAX) || (uInputSize / sizeof(wchar_t) != wsTemp.GetSize())){
		DEBUG_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, "The input Unicode string is too long"_rcs);
	}
	const auto uOutputSizeMax = uInputSize * 2;
	if((uOutputSizeMax > ULONG_MAX) || (uOutputSizeMax / 2 != uInputSize)){
		DEBUG_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, "The output ANSI string requires more memory than ULONG_MAX bytes"_rcs);
	}
	const auto uThirdOffset = uPos + uOutputSizeMax;
	if(uThirdOffset < uPos){
		throw std::bad_array_new_length();
	}
	const auto uOldSize = ansDst.GetSize();
	const auto pchWrite = ansDst.X_ChopAndSplice(uPos, uPos, 0, uThirdOffset);
	ULONG ulConvertedSize;
	const auto lStatus = ::RtlUnicodeToMultiByteN(pchWrite, uOutputSizeMax, &ulConvertedSize, wsTemp.GetBegin(), uInputSize);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "RtlUnicodeToMultiByteN"_rcs);
	}
	CopyN(pchWrite + ulConvertedSize, pchWrite + uOutputSizeMax, uOldSize - uPos);
	ansDst.X_SetSize(uOldSize + ulConvertedSize);
}

}
