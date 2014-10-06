// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"
#include "../Utilities/MinMax.hpp"
using namespace MCF;

namespace MCF {

namespace Impl {
	template<>
	void UnicodeConv<char, StringEncoding::ANSI>::operator()(
		UnifiedString &ucsUnified,
		const StringObserver<char> &soSrc
	) const {
		const auto uMoreSize = (soSrc.GetSize() + 1) / 2;
		const auto pWrite = ucsUnified.ResizeMore(uMoreSize);
		const auto nWritten = Max(
			::MultiByteToWideChar(
				CP_ACP, 0, soSrc.GetBegin(), (int)soSrc.GetLength(),
				pWrite, (int)uMoreSize
			), 0
		);
		ucsUnified.Resize((std::size_t)(pWrite + nWritten - ucsUnified.GetData()));
	}
	template<>
	void UnicodeConv<char, StringEncoding::ANSI>::operator()(
		String<char, StringEncoding::ANSI> &strDst,
		const UnifiedString &ucsUnified
	) const {
		const auto uMoreSize = ucsUnified.GetSize() * 2;
		const auto pWrite = strDst.ResizeMore(uMoreSize);
		const auto nWritten = Max(
			::WideCharToMultiByte(
				CP_ACP, 0, ucsUnified.GetBegin(), (int)ucsUnified.GetSize(),
				pWrite, (int)uMoreSize,
				nullptr, nullptr
			), 0
		);
		strDst.Resize((std::size_t)(pWrite + nWritten - strDst.GetData()));
	}

	template<>
	void UnicodeConv<wchar_t, StringEncoding::UTF16>::operator()(
		UnifiedString &ucsUnified,
		const StringObserver<wchar_t> &soSrc
	) const {
		const auto uSize = soSrc.GetSize();
		CopyN(ucsUnified.ResizeMore(uSize), soSrc.GetBegin(), uSize);
	}
	template<>
	void UnicodeConv<wchar_t, StringEncoding::UTF16>::operator()(
		String<wchar_t, StringEncoding::UTF16> &strDst,
		const UnifiedString &ucsUnified
	) const {
		const auto uSize = ucsUnified.GetSize();
		CopyN(strDst.ResizeMore(uSize), ucsUnified.GetBegin(), uSize);
	}

	// https://en.wikipedia.org/wiki/UTF-8
	template<>
	void UnicodeConv<char, StringEncoding::UTF8>::operator()(
		UnifiedString &ucsUnified,
		const StringObserver<char> &soSrc
	) const {
		// UTF-8 转 UTF-16。
		ucsUnified.ReserveMore(soSrc.GetSize() * 3);

		auto pchCur = soSrc.GetBegin();
		const auto pchEnd = soSrc.GetEnd();
		while(pchCur != pchEnd){
			std::uint32_t u32CodePoint = (std::uint8_t)*(pchCur++);
			if((u32CodePoint & 0x80u) == 0){
				// ASCII 字符。
				ucsUnified.Push(u32CodePoint);
				continue;
			}
			// 这个值是该码点剩余的字节数，不包含刚刚读取的。
			const std::size_t uAddnlBytes = CountLeadingZeroes(~(u32CodePoint << (BITS_OF(unsigned) - 7)) | 1);
			// UTF-8 理论上最长可以编码 6 个字符，但是标准化以后只能使用 4 个。
			if(uAddnlBytes - 1 > 2){ // 0 - 3
				// 按照 ISO-8859-1 映射到 U+0080 - U+00FF。
				ucsUnified.Push(u32CodePoint);
				continue;
			}
			if((std::size_t)(pchEnd - pchCur) < uAddnlBytes){
				--pchCur;
				while(pchCur != pchEnd){
					ucsUnified.Push((std::uint8_t)*(pchCur++));
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
					ucsUnified.Push((std::uint8_t)*(pchCur++));
				}
				continue;
			}
			if(u32CodePoint > 0x10FFFFu){
				// 无效的 UTF-32 码点。
				u32CodePoint = 0xFFFDu;
			}
			if(u32CodePoint <= 0xFFFFu){
				// 单个的 UTF-16 字符。
				ucsUnified.Push(u32CodePoint);
				continue;
			}
			// 编码成代理对。
			u32CodePoint -= 0x10000u;
			ucsUnified.Push((u32CodePoint >> 10)   | 0xD800u);
			ucsUnified.Push((u32CodePoint & 0x3FFu) | 0xDC00u);
		}
	}
	template<>
	void UnicodeConv<char, StringEncoding::UTF8>::operator()(
		String<char, StringEncoding::UTF8> &strDst,
		const UnifiedString &ucsUnified
	) const {
		// UTF-16 转 UTF-8。
		strDst.ReserveMore(ucsUnified.GetSize());

		auto pucCur = ucsUnified.GetBegin();
		const auto pucEnd = ucsUnified.GetEnd();
		while(pucCur != pucEnd){
			std::uint32_t u32CodePoint = (std::uint16_t)*(pucCur++);
			if(u32CodePoint - 0xD800u <= 0x7FFu){
				// 接受一个代理。
				u32CodePoint -= 0xD800u;
				if(u32CodePoint > 0x3FFu){
					// 这是个末代理。
					u32CodePoint = 0xFFFDu;
					goto jEncode;
				}
				if(pucCur == pucEnd){
					// 位于结尾的首代理。
					u32CodePoint = 0xFFFDu;
					goto jEncode;
				}
				const std::uint32_t u32Next = (std::uint16_t)*(pucCur++) - 0xDC00u;
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
		UnifiedString &ucsUnified,
		const StringObserver<char16_t> &soSrc
	) const {
		const auto uSize = soSrc.GetSize();
		CopyN(ucsUnified.ResizeMore(uSize), soSrc.GetBegin(), uSize);
	}
	template<>
	void UnicodeConv<char16_t, StringEncoding::UTF16>::operator()(
		String<char16_t, StringEncoding::UTF16> &strDst,
		const UnifiedString &ucsUnified
	) const {
		const auto uSize = strDst.GetSize();
		CopyN(strDst.ResizeMore(uSize), ucsUnified.GetBegin(), uSize);
	}

	// https://en.wikipedia.org/wiki/UTF-16
	template<>
	void UnicodeConv<char32_t, StringEncoding::UTF32>::operator()(
		UnifiedString &ucsUnified,
		const StringObserver<char32_t> &soSrc
	) const {
		// UTF-32 转 UTF-16。
		ucsUnified.ReserveMore(soSrc.GetSize());

		for(auto u32CodePoint : soSrc){
			if(u32CodePoint > 0x10FFFFu){
				// 无效的 UTF-32 码点。
				u32CodePoint = 0xFFFDu;
			}
			if(u32CodePoint <= 0xFFFFu){
				// 单个的 UTF-16 字符。
				ucsUnified.Push(u32CodePoint);
				continue;
			}
			// 编码成代理对。
			u32CodePoint -= 0x10000u;
			ucsUnified.Push((u32CodePoint >> 10)   | 0xD800u);
			ucsUnified.Push((u32CodePoint & 0x3FFu) | 0xDC00u);
		}
	}
	template<>
	void UnicodeConv<char32_t, StringEncoding::UTF32>::operator()(
		String<char32_t, StringEncoding::UTF32> &strDst,
		const UnifiedString &ucsUnified
	) const {
		// UTF-16 转 UTF-32。
		strDst.ReserveMore(ucsUnified.GetSize());

		auto pucCur = ucsUnified.GetBegin();
		const auto pucEnd = ucsUnified.GetEnd();
		while(pucCur != pucEnd){
			std::uint32_t u32CodePoint = (std::uint16_t)*(pucCur++);
			if(u32CodePoint - 0xD800u <= 0x7FFu){
				// 接受一个代理。
				u32CodePoint -= 0xD800u;
				if(u32CodePoint > 0x3FFu){
					// 这是个末代理。
					u32CodePoint = 0xFFFDu;
					goto jDone;
				}
				if(pucCur == pucEnd){
					// 位于结尾的首代理。
					u32CodePoint = 0xFFFDu;
					goto jDone;
				}
				const std::uint32_t u32Next = (std::uint16_t)*(pucCur++) - 0xDC00u;
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

}
