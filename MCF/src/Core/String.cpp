// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "String.hpp"
#include "Exception.hpp"
#include <winternl.h>
#include <ntstatus.h>
#include <MCFCRT/ext/utf.h>

extern "C" {

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS RtlMultiByteToUnicodeN(wchar_t *pwcBuffer, ULONG ulBufferSize, ULONG *pulBytesMax, const char *pchMultiByteString, ULONG ulMultiByteStringSize) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS RtlUnicodeToMultiByteN(char *pchBuffer, ULONG ulBufferSize, ULONG *pulBytesMax, const wchar_t *pwcUnicodeString, ULONG ulUnicodeStringSize) noexcept;

}

namespace MCF {

template class String<StringType::kNarrow>;
template class String<StringType::kWide>;
template class String<StringType::kUtf8>;
template class String<StringType::kUtf16>;
template class String<StringType::kUtf32>;
template class String<StringType::kCesu8>;
template class String<StringType::kAnsi>;
template class String<StringType::kModifiedUtf8>;

// UTF-8
template<>
__attribute__((__flatten__))
void NarrowString::UnifyAppend(Utf16String &u16sDst, const NarrowStringView &svSrc){
	const auto pc16WriteBegin = u16sDst.ResizeMore(svSrc.GetSize());
	try {
		auto pc16Write = pc16WriteBegin;
		auto pchRead = svSrc.GetBegin();
		const auto pchReadEnd = svSrc.GetEnd();
		while(pchRead < pchReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf8(&pchRead, pchReadEnd, false, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"NarrowString: _MCFCRT_DecodeUtf8() 失败。"));
			}
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf16(&pc16Write, c32CodePoint, true);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"NarrowString: _MCFCRT_UncheckedEncodeUtf16() 失败。"));
			}
		}
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16Write));
	} catch(...){
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16WriteBegin));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void NarrowString::DeunifyAppend(NarrowString &strDst, const Utf16StringView &u16svSrc){
	const auto pchWriteBegin = strDst.ResizeMore(Impl_CheckedSizeArithmetic::Mul(3, u16svSrc.GetSize()));
	try {
		auto pchWrite = pchWriteBegin;
		auto pc16Read = u16svSrc.GetBegin();
		const auto pc16ReadEnd = u16svSrc.GetEnd();
		while(pc16Read < pc16ReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf16(&pc16Read, pc16ReadEnd, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"NarrowString: _MCFCRT_DecodeUtf16() 失败。"));
			}
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf8(&pchWrite, c32CodePoint, true, false, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"NarrowString: _MCFCRT_UncheckedEncodeUtf8() 失败。"));
			}
		}
		strDst.Pop(static_cast<std::size_t>(strDst.GetEnd() - pchWrite));
	} catch(...){
		strDst.Pop(static_cast<std::size_t>(strDst.GetEnd() - pchWriteBegin));
		throw;
	}
}

template<>
__attribute__((__flatten__))
void NarrowString::UnifyAppend(Utf32String &u32sDst, const NarrowStringView &svSrc){
	const auto pc32WriteBegin = u32sDst.ResizeMore(svSrc.GetSize());
	try {
		auto pc32Write = pc32WriteBegin;
		auto pchRead = svSrc.GetBegin();
		const auto pchReadEnd = svSrc.GetEnd();
		while(pchRead < pchReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf8(&pchRead, pchReadEnd, false, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"NarrowString: _MCFCRT_DecodeUtf8() 失败。"));
			}
			*(pc32Write++) = c32CodePoint;
		}
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32Write));
	} catch(...){
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32WriteBegin));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void NarrowString::DeunifyAppend(NarrowString &strDst, const Utf32StringView &u32svSrc){
	const auto pchWriteBegin = strDst.ResizeMore(Impl_CheckedSizeArithmetic::Mul(4, u32svSrc.GetSize()));
	try {
		auto pchWrite = pchWriteBegin;
		auto pc32Read = u32svSrc.GetBegin();
		const auto pc32ReadEnd = u32svSrc.GetEnd();
		while(pc32Read < pc32ReadEnd){
			auto c32CodePoint = *(pc32Read++);
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf8(&pchWrite, c32CodePoint, true, false, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"NarrowString: _MCFCRT_EncodeUtf8() 失败。"));
			}
		}
		strDst.Pop(static_cast<std::size_t>(strDst.GetEnd() - pchWrite));
	} catch(...){
		strDst.Pop(static_cast<std::size_t>(strDst.GetEnd() - pchWriteBegin));
		throw;
	}
}

// UTF-16
template<>
__attribute__((__flatten__))
void WideString::UnifyAppend(Utf16String &u16sDst, const WideStringView &svSrc){
//	u16sDst.Append(reinterpret_cast<const char16_t *>(svSrc.GetBegin()), svSrc.GetSize());
}
template<>
__attribute__((__flatten__))
void WideString::DeunifyAppend(WideString &strDst, const Utf16StringView &u16svSrc){
//	strDst.Append(reinterpret_cast<const wchar_t *>(u16svSrc.GetBegin()), u16svSrc.GetSize());
}

template<>
__attribute__((__flatten__))
void WideString::UnifyAppend(Utf32String &u32sDst, const WideStringView &svSrc){
//	u32sDst.ReserveMore(svSrc.GetSize());
//	Convert(u32sDst, MakeUtf16Decoder(MakeStringSource(svSrc)));
}
template<>
__attribute__((__flatten__))
void WideString::DeunifyAppend(WideString &strDst, const Utf32StringView &u32svSrc){
//	strDst.ReserveMore(u32svSrc.GetSize());
//	Convert(strDst, MakeUtf16Encoder(MakeStringSource(u32svSrc)));
}

// UTF-8
template<>
__attribute__((__flatten__))
void Utf8String::UnifyAppend(Utf16String &u16sDst, const Utf8StringView &svSrc){
	const auto pc16WriteBegin = u16sDst.ResizeMore(svSrc.GetSize());
	try {
		auto pc16Write = pc16WriteBegin;
		auto pchRead = svSrc.GetBegin();
		const auto pchReadEnd = svSrc.GetEnd();
		while(pchRead < pchReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf8(&pchRead, pchReadEnd, false, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_DecodeUtf8() 失败。"));
			}
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf16(&pc16Write, c32CodePoint, true);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_UncheckedEncodeUtf16() 失败。"));
			}
		}
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16Write));
	} catch(...){
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16WriteBegin));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void Utf8String::DeunifyAppend(Utf8String &strDst, const Utf16StringView &u16svSrc){
	const auto pchWriteBegin = strDst.ResizeMore(Impl_CheckedSizeArithmetic::Mul(3, u16svSrc.GetSize()));
	try {
		auto pchWrite = pchWriteBegin;
		auto pc16Read = u16svSrc.GetBegin();
		const auto pc16ReadEnd = u16svSrc.GetEnd();
		while(pc16Read < pc16ReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf16(&pc16Read, pc16ReadEnd, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_DecodeUtf16() 失败。"));
			}
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf8(&pchWrite, c32CodePoint, true, false, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_UncheckedEncodeUtf8() 失败。"));
			}
		}
		strDst.Pop(static_cast<std::size_t>(strDst.GetEnd() - pchWrite));
	} catch(...){
		strDst.Pop(static_cast<std::size_t>(strDst.GetEnd() - pchWriteBegin));
		throw;
	}
}

template<>
__attribute__((__flatten__))
void Utf8String::UnifyAppend(Utf32String &u32sDst, const Utf8StringView &svSrc){
	const auto pc32WriteBegin = u32sDst.ResizeMore(svSrc.GetSize());
	try {
		auto pc32Write = pc32WriteBegin;
		auto pchRead = svSrc.GetBegin();
		const auto pchReadEnd = svSrc.GetEnd();
		while(pchRead < pchReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf8(&pchRead, pchReadEnd, false, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_DecodeUtf8() 失败。"));
			}
			*(pc32Write++) = c32CodePoint;
		}
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32Write));
	} catch(...){
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32WriteBegin));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void Utf8String::DeunifyAppend(Utf8String &strDst, const Utf32StringView &u32svSrc){
	const auto pchWriteBegin = strDst.ResizeMore(Impl_CheckedSizeArithmetic::Mul(4, u32svSrc.GetSize()));
	try {
		auto pchWrite = pchWriteBegin;
		auto pc32Read = u32svSrc.GetBegin();
		const auto pc32ReadEnd = u32svSrc.GetEnd();
		while(pc32Read < pc32ReadEnd){
			auto c32CodePoint = *(pc32Read++);
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf8(&pchWrite, c32CodePoint, true, false, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_EncodeUtf8() 失败。"));
			}
		}
		strDst.Pop(static_cast<std::size_t>(strDst.GetEnd() - pchWrite));
	} catch(...){
		strDst.Pop(static_cast<std::size_t>(strDst.GetEnd() - pchWriteBegin));
		throw;
	}
}

// UTF-16
template<>
__attribute__((__flatten__))
void Utf16String::UnifyAppend(Utf16String &u16sDst, const Utf16StringView &svSrc){
//	u16sDst.Append(svSrc.GetBegin(), svSrc.GetSize());
}
template<>
__attribute__((__flatten__))
void Utf16String::DeunifyAppend(Utf16String &strDst, const Utf16StringView &u16svSrc){
//	strDst.Append(u16svSrc.GetBegin(), u16svSrc.GetSize());
}

template<>
__attribute__((__flatten__))
void Utf16String::UnifyAppend(Utf32String &u32sDst, const Utf16StringView &svSrc){
//	u32sDst.ReserveMore(svSrc.GetSize());
//	Convert(u32sDst, MakeUtf16Decoder(MakeStringSource(svSrc)));
}
template<>
__attribute__((__flatten__))
void Utf16String::DeunifyAppend(Utf16String &strDst, const Utf32StringView &u32svSrc){
//	strDst.ReserveMore(u32svSrc.GetSize());
//	Convert(strDst, MakeUtf16Encoder(MakeStringSource(u32svSrc)));
}

// UTF-32
template<>
__attribute__((__flatten__))
void Utf32String::UnifyAppend(Utf16String &u16sDst, const Utf32StringView &svSrc){
//	u16sDst.ReserveMore(svSrc.GetSize());
//	Convert(u16sDst, MakeUtf16Encoder(MakeStringSource(svSrc)));
}
template<>
__attribute__((__flatten__))
void Utf32String::DeunifyAppend(Utf32String &strDst, const Utf16StringView &u16svSrc){
//	strDst.ReserveMore(u16svSrc.GetSize());
//	Convert(strDst, MakeUtf16Decoder(MakeStringSource(u16svSrc)));
}

template<>
__attribute__((__flatten__))
void Utf32String::UnifyAppend(Utf32String &u32sDst, const Utf32StringView &svSrc){
//	u32sDst.Append(svSrc.GetBegin(), svSrc.GetSize());
}
template<>
__attribute__((__flatten__))
void Utf32String::DeunifyAppend(Utf32String &strDst, const Utf32StringView &u32svSrc){
//	strDst.Append(u32svSrc.GetBegin(), u32svSrc.GetSize());
}

// CESU-8
template<>
__attribute__((__flatten__))
void Cesu8String::UnifyAppend(Utf16String &u16sDst, const Cesu8StringView &svSrc){
//	u16sDst.ReserveMore(svSrc.GetSize());
//	Convert(u16sDst, MakeCesu8Decoder(MakeStringSource(svSrc)));
}
template<>
__attribute__((__flatten__))
void Cesu8String::DeunifyAppend(Cesu8String &strDst, const Utf16StringView &u16svSrc){
//	strDst.ReserveMore(u16svSrc.GetSize() * 3);
//	Convert(strDst, MakeUtf8Encoder(MakeStringSource(u16svSrc)));
}

template<>
__attribute__((__flatten__))
void Cesu8String::UnifyAppend(Utf32String &u32sDst, const Cesu8StringView &svSrc){
//	u32sDst.ReserveMore(svSrc.GetSize());
//	Convert(u32sDst, MakeUtf16Decoder(MakeCesu8Decoder(MakeStringSource(svSrc))));
}
template<>
__attribute__((__flatten__))
void Cesu8String::DeunifyAppend(Cesu8String &strDst, const Utf32StringView &u32svSrc){
//	strDst.ReserveMore(u32svSrc.GetSize() * 2);
//	Convert(strDst, MakeUtf8Encoder(MakeUtf16Encoder(MakeStringSource(u32svSrc))));
}

// ANSI
template<>
__attribute__((__flatten__))
void AnsiString::UnifyAppend(Utf16String &u16sDst, const AnsiStringView &svSrc){
	const auto uInputSize = svSrc.GetSize() * sizeof(char);
	if(uInputSize > ULONG_MAX){
		MCF_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, Rcntws::View(L"AnsiString: 输入的 ANSI 字符串太长。"));
	}
	const auto uOutputSizeMax = svSrc.GetSize() * sizeof(wchar_t);
	if((uOutputSizeMax > ULONG_MAX) || (uOutputSizeMax / sizeof(wchar_t) != svSrc.GetSize())){
		MCF_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, Rcntws::View(L"AnsiString: 输出的 UTF-16 字符串太长。"));
	}
	const auto pchWrite = u16sDst.ResizeMore(uOutputSizeMax / sizeof(wchar_t));
	try {
		ULONG ulConvertedSize;
		const auto lStatus = ::RtlMultiByteToUnicodeN(reinterpret_cast<wchar_t *>(pchWrite), (DWORD)uOutputSizeMax, &ulConvertedSize, svSrc.GetBegin(), (DWORD)uInputSize);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"AnsiString: RtlMultiByteToUnicodeN() 失败。"));
		}
		u16sDst.Pop(uOutputSizeMax / sizeof(wchar_t) - ulConvertedSize / sizeof(wchar_t));
	} catch(...){
		u16sDst.Pop(uOutputSizeMax / sizeof(wchar_t));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void AnsiString::DeunifyAppend(AnsiString &strDst, const Utf16StringView &u16svSrc){
	const auto uInputSize = u16svSrc.GetSize() * sizeof(wchar_t);
	if((uInputSize > ULONG_MAX) || (uInputSize / sizeof(wchar_t) != u16svSrc.GetSize())){
		MCF_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, Rcntws::View(L"AnsiString: 输入的 UTF-16 字符串太长。"));
	}
	const auto uOutputSizeMax = u16svSrc.GetSize() * 2 * sizeof(char);
	if((uOutputSizeMax > ULONG_MAX) || (uOutputSizeMax / (2 * sizeof(char)) != u16svSrc.GetSize())){
		MCF_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, Rcntws::View(L"AnsiString: 输出的 ANSI 字符串太长。"));
	}
	const auto pchWrite = strDst.ResizeMore(uOutputSizeMax / sizeof(char));
	try {
		ULONG ulConvertedSize;
		const auto lStatus = ::RtlUnicodeToMultiByteN(pchWrite, (DWORD)uOutputSizeMax, &ulConvertedSize, reinterpret_cast<const wchar_t *>(u16svSrc.GetBegin()), (DWORD)uInputSize);
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"AnsiString: RtlUnicodeToMultiByteN() 失败。"));
		}
		strDst.Pop(uOutputSizeMax / sizeof(char) - ulConvertedSize / sizeof(char));
	} catch(...){
		strDst.Pop(uOutputSizeMax / sizeof(char));
		throw;
	}
}

template<>
__attribute__((__flatten__))
void AnsiString::UnifyAppend(Utf32String &u32sDst, const AnsiStringView &svSrc){
	Utf16String u16sTemp;
	UnifyAppend(u16sTemp, svSrc);
	Utf16String::UnifyAppend(u32sDst, u16sTemp);
}
template<>
__attribute__((__flatten__))
void AnsiString::DeunifyAppend(AnsiString &strDst, const Utf32StringView &u32svSrc){
	Utf16String u16sTemp;
	Utf16String::DeunifyAppend(u16sTemp, u32svSrc);
	DeunifyAppend(strDst, u16sTemp);
}

// Modified UTF-8
template<>
__attribute__((__flatten__))
void ModifiedUtf8String::UnifyAppend(Utf16String &u16sDst, const ModifiedUtf8StringView &svSrc){
//	u16sDst.ReserveMore(svSrc.GetSize());
//	Convert(u16sDst, MakeUtf8Decoder(MakeStringSource(svSrc)));
}
template<>
__attribute__((__flatten__))
void ModifiedUtf8String::DeunifyAppend(ModifiedUtf8String &strDst, const Utf16StringView &u16svSrc){
//	strDst.ReserveMore(u16svSrc.GetSize() * 3);
//	Convert(strDst, MakeModifiedUtf8Encoder(MakeStringSource(u16svSrc)));
}

template<>
__attribute__((__flatten__))
void ModifiedUtf8String::UnifyAppend(Utf32String &u32sDst, const ModifiedUtf8StringView &svSrc){
//	u32sDst.ReserveMore(svSrc.GetSize());
//	Convert(u32sDst, MakeUtf16Decoder(MakeUtf8Decoder(MakeStringSource(svSrc))));
}
template<>
__attribute__((__flatten__))
void ModifiedUtf8String::DeunifyAppend(ModifiedUtf8String &strDst, const Utf32StringView &u32svSrc){
//	strDst.ReserveMore(u32svSrc.GetSize() * 2);
//	Convert(strDst, MakeModifiedUtf8Encoder(MakeUtf16Encoder(MakeStringSource(u32svSrc))));
}

}
