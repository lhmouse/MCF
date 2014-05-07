// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"
#include "Utilities.hpp"
#include <limits>

namespace MCF {

template class String<char,		StringEncoding::ANSI>;
template class String<wchar_t,	StringEncoding::UTF16>;

template class String<char,		StringEncoding::UTF8>;
template class String<char16_t,	StringEncoding::UTF16>;
template class String<char32_t,	StringEncoding::UTF32>;

template<>
VVector<wchar_t> AnsiString::xUnify() const {
	VVector<wchar_t> vecUnified;
	const int nUnifiedLen = ::MultiByteToWideChar(CP_ACP, 0, GetBegin(), (int)GetLength(), nullptr, 0);
	if(nUnifiedLen > 0){
		vecUnified.Resize((std::size_t)(unsigned int)nUnifiedLen);
		vecUnified.Resize((std::size_t)(unsigned int)::MultiByteToWideChar(
			CP_ACP,
			0,
			GetBegin(),
			(int)GetLength(),
			vecUnified.GetData(),
			nUnifiedLen
		));
	}
	return std::move(vecUnified);
}
template<>
void AnsiString::xDeunifyAppend(const VVector<wchar_t> &vecUnified){
	const int nDeunifiedLen = ::WideCharToMultiByte(CP_ACP, 0, vecUnified.GetBegin(), (int)vecUnified.GetSize(), nullptr, 0, nullptr, nullptr);
	if(nDeunifiedLen > 0){
		const auto uOldLength = GetLength();
		Resize(uOldLength + (std::size_t)(unsigned int)nDeunifiedLen);
		Resize(uOldLength + (std::size_t)(unsigned int)::WideCharToMultiByte(
			CP_ACP,
			0,
			vecUnified.GetBegin(),
			(int)vecUnified.GetSize(),
			GetData(),
			nDeunifiedLen,
			nullptr,
			nullptr
		));
	}
}

template<>
VVector<wchar_t> WideString::xUnify() const {
	return VVector<wchar_t>(GetBegin(), GetEnd());
}
template<>
void WideString::xDeunifyAppend(const VVector<wchar_t> &vecUnified){
	Append(vecUnified.GetData(), vecUnified.GetSize());
}

// https://en.wikipedia.org/wiki/UTF-8
template<>
VVector<wchar_t> Utf8String::xUnify() const {
	// UTF-8 转 UTF-16。
	VVector<wchar_t> vecUnified;
	vecUnified.Reserve(GetSize() * 3);

	auto pchCur = GetBegin();
	const auto pchEnd = GetEnd();
	while(pchCur != pchEnd){
		std::uint32_t u32CodePoint = (std::uint8_t)*(pchCur++);
		if((u32CodePoint & 0x80u) == 0){
			// ASCII 字符。
			vecUnified.Push(u32CodePoint);
			continue;
		}
		// 这个值是该码点剩余的字节数，不包含刚刚读取的。
		const auto uAddnlBytes = (std::size_t)__builtin_clzl(~(u32CodePoint << (std::numeric_limits<unsigned int>::digits - 7)) | 1);
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

		u32CodePoint &= 0x7Fu >> uAddnlBytes;
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

	return std::move(vecUnified);
}
template<>
void Utf8String::xDeunifyAppend(const VVector<wchar_t> &vecUnified){
	// UTF-16 转 UTF-8。
	Reserve(GetSize() + vecUnified.GetSize());

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
		const auto uAddnlBytes = (std::size_t)(29 - __builtin_clzl(u32CodePoint | 1)) / 5;
		if(uAddnlBytes == 0){
			ASSERT(u32CodePoint <= 0x7Fu);

			Push(u32CodePoint);
			continue;
		}
		u32CodePoint |= (std::uint32_t)0xFFFFFF80u << (uAddnlBytes * 5);
		Push(u32CodePoint >> (uAddnlBytes * 6));
		for(std::size_t i = uAddnlBytes; i != 0; --i){
			Push(((u32CodePoint >> (i * 6 - 6)) & 0x3Fu) | 0x80u);
		}
	}
}

template<>
VVector<wchar_t> Utf16String::xUnify() const {
	VVector<wchar_t> vecUnified;
	vecUnified.Resize(vecUnified.GetSize() + GetSize());
	std::copy(GetBegin(), GetEnd(), vecUnified.GetEnd() - GetSize());
	return std::move(vecUnified);
}
template<>
void Utf16String::xDeunifyAppend(const VVector<wchar_t> &vecUnified){
	Resize(GetSize() + vecUnified.GetSize());
	std::copy(vecUnified.GetBegin(), vecUnified.GetEnd(), GetEnd() - GetSize());
}

// https://en.wikipedia.org/wiki/UTF-16
template<>
VVector<wchar_t> Utf32String::xUnify() const {
	// UTF-32 转 UTF-16。
	VVector<wchar_t> vecUnified;
	vecUnified.Reserve(GetSize());

	for(auto u32CodePoint : *this){
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

	return std::move(vecUnified);
}
template<>
void Utf32String::xDeunifyAppend(const VVector<wchar_t> &vecUnified){
	// UTF-16 转 UTF-32。
	Reserve(GetSize() + vecUnified.GetSize());

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
		Push(u32CodePoint);
	}
}

}
