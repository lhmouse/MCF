// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"
#include "Utilities.hpp"
#include "../Serialization/Serdes.hpp"
#include "../Thread/ReaderWriterLock.hpp"
#include <unordered_map>
using namespace MCF;

namespace MCF {

namespace Impl {
	template<>
	void UnicodeConv<char, StringEncoding::ANSI>::operator()(
		VVector<wchar_t> &vecUnified,
		const StringObserver<char> &soSrc
	) const {
		const auto uMoreSize = (soSrc.GetSize() + 1) / 2;
		const auto pWrite = vecUnified.ResizeMore(uMoreSize);
		const auto nWritten = Max(
			::MultiByteToWideChar(
				CP_ACP, 0, soSrc.GetBegin(), (int)soSrc.GetLength(),
				pWrite, (int)uMoreSize
			), 0
		);
		vecUnified.Resize((std::size_t)(pWrite + nWritten - vecUnified.GetData()));
	}
	template<>
	void UnicodeConv<char, StringEncoding::ANSI>::operator()(
		String<char, StringEncoding::ANSI> &strDst,
		const VVector<wchar_t> &vecUnified
	) const {
		const auto uMoreSize = vecUnified.GetSize() * 2;
		const auto pWrite = strDst.ResizeMore(uMoreSize);
		const auto nWritten = Max(
			::WideCharToMultiByte(
				CP_ACP, 0, vecUnified.GetBegin(), (int)vecUnified.GetSize(),
				pWrite, (int)uMoreSize,
				nullptr, nullptr
			), 0
		);
		strDst.Resize((std::size_t)(pWrite + nWritten - strDst.GetData()));
	}

	template<>
	void UnicodeConv<wchar_t, StringEncoding::UTF16>::operator()(
		VVector<wchar_t> &vecUnified,
		const StringObserver<wchar_t> &soSrc
	) const {
		const auto uSize = soSrc.GetSize();
		CopyN(vecUnified.ResizeMore(uSize), soSrc.GetBegin(), uSize);
	}
	template<>
	void UnicodeConv<wchar_t, StringEncoding::UTF16>::operator()(
		String<wchar_t, StringEncoding::UTF16> &strDst,
		const VVector<wchar_t> &vecUnified
	) const {
		const auto uSize = vecUnified.GetSize();
		CopyN(strDst.ResizeMore(uSize), vecUnified.GetBegin(), uSize);
	}

	// https://en.wikipedia.org/wiki/UTF-8
	template<>
	void UnicodeConv<char, StringEncoding::UTF8>::operator()(
		VVector<wchar_t> &vecUnified,
		const StringObserver<char> &soSrc
	) const {
		// UTF-8 转 UTF-16。
		vecUnified.ReserveMore(soSrc.GetSize() * 3);

		auto pchCur = soSrc.GetBegin();
		const auto pchEnd = soSrc.GetEnd();
		while(pchCur != pchEnd){
			std::uint32_t u32CodePoint = (std::uint8_t)*(pchCur++);
			if((u32CodePoint & 0x80u) == 0){
				// ASCII 字符。
				vecUnified.Push(u32CodePoint);
				continue;
			}
			// 这个值是该码点剩余的字节数，不包含刚刚读取的。
			const std::size_t uAddnlBytes = CountLeadingZeroes(~(u32CodePoint << (BITS_OF(unsigned int) - 7)) | 1);
			// UTF-8 理论上最长可以编码 6 个字符，但是标准化以后只能使用 4 个。
			if(uAddnlBytes - 1 > 2){ // 0 - 3
				// 按照 ISO-8859-1 映射到 U+0080 - U+00FF。
				vecUnified.Push(u32CodePoint);
				continue;
			}
			if((std::size_t)(pchEnd - pchCur) < uAddnlBytes){
				--pchCur;
				while(pchCur != pchEnd){
					vecUnified.Push((std::uint8_t)*(pchCur++));
				}
				break;
			}

			u32CodePoint &= (0x7Fu >> uAddnlBytes);
			const auto pchCodeEnd = pchCur + uAddnlBytes;
			while(pchCur != pchCodeEnd){
				const auto uchCur = (std::uint8_t)*(pchCur++);
				if((uchCur & 0xC0u) != 0x80u){
					// 编码错误。
					break;
				}
				u32CodePoint <<= 6;
				u32CodePoint |= (uchCur & 0x3Fu);
			}
			if(pchCur != pchCodeEnd){
				// 如果编码出错，映射到 U+0080 - U+00FF。
				pchCur = pchCodeEnd - uAddnlBytes - 1;
				while(pchCur < pchCodeEnd){
					vecUnified.Push((std::uint8_t)*(pchCur++));
				}
				continue;
			}
			if(u32CodePoint > 0x10FFFFu){
				// 无效的 UTF-32 码点。
				u32CodePoint = 0xFFFDu;
			}
			if(u32CodePoint <= 0xFFFFu){
				// 单个的 UTF-16 字符。
				vecUnified.Push(u32CodePoint);
				continue;
			}
			// 编码成代理对。
			u32CodePoint -= 0x10000u;
			vecUnified.Push((u32CodePoint >> 10)   | 0xD800u);
			vecUnified.Push((u32CodePoint & 0x3FFu) | 0xDC00u);
		}
	}
	template<>
	void UnicodeConv<char, StringEncoding::UTF8>::operator()(
		String<char, StringEncoding::UTF8> &strDst,
		const VVector<wchar_t> &vecUnified
	) const {
		// UTF-16 转 UTF-8。
		strDst.ReserveMore(vecUnified.GetSize());

		auto pwcCur = vecUnified.GetBegin();
		const auto pwcEnd = vecUnified.GetEnd();
		while(pwcCur != pwcEnd){
			std::uint32_t u32CodePoint = (std::uint16_t)*(pwcCur++);
			if(u32CodePoint - 0xD800u <= 0x7FFu){
				// 接受一个代理。
				u32CodePoint -= 0xD800u;
				if(u32CodePoint > 0x3FFu){
					// 这是个末代理。
					u32CodePoint = 0xFFFDu;
					goto jEncode;
				}
				if(pwcCur == pwcEnd){
					// 位于结尾的首代理。
					u32CodePoint = 0xFFFDu;
					goto jEncode;
				}
				const std::uint32_t u32Next = (std::uint16_t)*(pwcCur++) - 0xDC00u;
				if(u32Next > 0x3FFu){
					// 末代理无效。
					u32CodePoint = 0xFFFDu;
					goto jEncode;
				}
				// 将代理对拼成一个码点。
				u32CodePoint <<= 10;
				u32CodePoint |= u32Next;
				u32CodePoint += 0x10000u;
				if(u32CodePoint > 0x10FFFFu){
					// 无效的 UTF-32 码点。
					u32CodePoint = 0xFFFDu;
				}
			}

		jEncode:
			// 这个值是该码点的总字节数减一。
			const auto uAddnlBytes = (std::size_t)(29 - CountLeadingZeroes(u32CodePoint | 1)) / 5;
			if(uAddnlBytes == 0){
				ASSERT(u32CodePoint <= 0x7Fu);

				strDst.Push(u32CodePoint);
				continue;
			}
			u32CodePoint |= (std::uint32_t)0xFFFFFF80u << (uAddnlBytes * 5);
			strDst.Push(u32CodePoint >> (uAddnlBytes * 6));
			for(std::size_t i = uAddnlBytes; i != 0; --i){
				strDst.Push(((u32CodePoint >> (i * 6 - 6)) & 0x3Fu) | 0x80u);
			}
		}
	}

	template<>
	void UnicodeConv<char16_t, StringEncoding::UTF16>::operator()(
		VVector<wchar_t> &vecUnified,
		const StringObserver<char16_t> &soSrc
	) const {
		const auto uSize = soSrc.GetSize();
		CopyN(vecUnified.ResizeMore(uSize), soSrc.GetBegin(), uSize);
	}
	template<>
	void UnicodeConv<char16_t, StringEncoding::UTF16>::operator()(
		String<char16_t, StringEncoding::UTF16> &strDst,
		const VVector<wchar_t> &vecUnified
	) const {
		const auto uSize = strDst.GetSize();
		CopyN(strDst.ResizeMore(uSize), vecUnified.GetBegin(), uSize);
	}

	// https://en.wikipedia.org/wiki/UTF-16
	template<>
	void UnicodeConv<char32_t, StringEncoding::UTF32>::operator()(
		VVector<wchar_t> &vecUnified,
		const StringObserver<char32_t> &soSrc
	) const {
		// UTF-32 转 UTF-16。
		vecUnified.ReserveMore(soSrc.GetSize());

		for(auto u32CodePoint : soSrc){
			if(u32CodePoint > 0x10FFFFu){
				// 无效的 UTF-32 码点。
				u32CodePoint = 0xFFFDu;
			}
			if(u32CodePoint <= 0xFFFFu){
				// 单个的 UTF-16 字符。
				vecUnified.Push(u32CodePoint);
				continue;
			}
			// 编码成代理对。
			u32CodePoint -= 0x10000u;
			vecUnified.Push((u32CodePoint >> 10)   | 0xD800u);
			vecUnified.Push((u32CodePoint & 0x3FFu) | 0xDC00u);
		}
	}
	template<>
	void UnicodeConv<char32_t, StringEncoding::UTF32>::operator()(
		String<char32_t, StringEncoding::UTF32> &strDst,
		const VVector<wchar_t> &vecUnified
	) const {
		// UTF-16 转 UTF-32。
		strDst.ReserveMore(vecUnified.GetSize());

		auto pwcCur = vecUnified.GetBegin();
		const auto pwcEnd = vecUnified.GetEnd();
		while(pwcCur != pwcEnd){
			std::uint32_t u32CodePoint = (std::uint16_t)*(pwcCur++);
			if(u32CodePoint - 0xD800u <= 0x7FFu){
				// 接受一个代理。
				u32CodePoint -= 0xD800u;
				if(u32CodePoint > 0x3FFu){
					// 这是个末代理。
					u32CodePoint = 0xFFFDu;
					goto jDone;
				}
				if(pwcCur == pwcEnd){
					// 位于结尾的首代理。
					u32CodePoint = 0xFFFDu;
					goto jDone;
				}
				const std::uint32_t u32Next = (std::uint16_t)*(pwcCur++) - 0xDC00u;
				if(u32Next > 0x3FFu){
					// 末代理无效。
					u32CodePoint = 0xFFFDu;
					goto jDone;
				}
				// 将代理对拼成一个码点。
				u32CodePoint <<= 10;
				u32CodePoint |= u32Next;
				u32CodePoint += 0x10000u;
				if(u32CodePoint > 0x10FFFFu){
					// 无效的 UTF-32 码点。
					u32CodePoint = 0xFFFDu;
				}
			}

		jDone:
			strDst.Push(u32CodePoint);
		}
	}
}

template class String<char,		StringEncoding::ANSI>;
template class String<wchar_t,	StringEncoding::UTF16>;

template class String<char,		StringEncoding::UTF8>;
template class String<char16_t,	StringEncoding::UTF16>;
template class String<char32_t,	StringEncoding::UTF32>;

// 串行化。
void Serialize(StreamBuffer &sbufSink, const Utf8String &u8sSource){
	Serialize<unsigned long long>(sbufSink, u8sSource.GetSize());
	sbufSink.Insert(u8sSource.GetCStr(), u8sSource.GetSize());
}
void Deserialize(Utf8String &u8sSink, StreamBuffer &sbufSource){
	unsigned long long ullSize;
	Deserialize(ullSize, sbufSource);
	if(ullSize > std::numeric_limits<std::size_t>::max()){
		ThrowInvalidData();
	}
	// 万一 ullSize 是伪造的呢？
	u8sSink.Clear();
	while(ullSize){
		const int nChar = sbufSource.Get();
		if(nChar == -1){
			ThrowEndOfStream();
		}
		u8sSink.Push(nChar);
		--ullSize;
	}
}

// 字面量运算符。
#define DEFINE_LITERAL_OPERATOR(suffix, charType, encoding)	\
	const String<MACRO_TYPE(charType), StringEncoding::encoding> &	\
		operator"" ## suffix(const MACRO_TYPE(charType) *pchStr, std::size_t uLength)	\
	{	\
		static const auto pReaderWriterLock = ReaderWriterLock::Create();	\
		static std::unordered_map<	\
			const charType *,	\
			String<MACRO_TYPE(charType), StringEncoding::encoding>	\
		> mapStrings;	\
		\
		{	\
			const auto vReaderLock = pReaderWriterLock->GetReaderLock();	\
			auto itString = mapStrings.find(pchStr);	\
			if(itString != mapStrings.end()){	\
				return itString->second;	\
			}	\
		}	\
		\
		const auto vWriterLock = pReaderWriterLock->GetWriterLock();	\
		return mapStrings.emplace(	\
			std::piecewise_construct,	\
			std::make_tuple(pchStr),	\
			std::make_tuple(pchStr, uLength)	\
		).first->second;	\
	}

DEFINE_LITERAL_OPERATOR(_as,	char,		ANSI)
DEFINE_LITERAL_OPERATOR(_ws,	wchar_t,	UTF16)

DEFINE_LITERAL_OPERATOR(_u8s,	char,		UTF8)
DEFINE_LITERAL_OPERATOR(_u16s,	char16_t,	UTF16)
DEFINE_LITERAL_OPERATOR(_u32s,	char32_t,	UTF32)

}
