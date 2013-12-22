// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"

namespace MCF {
	template class GenericString<char,		String::ENC_UTF8>;
	template class GenericString<char,		String::ENC_ANSI>;
	template class GenericString<wchar_t,	String::ENC_UTF16>;

	template<>
	UNIFIED_CHAR_SEQ UTF8String::xUnify() const {
		UNIFIED_CHAR_SEQ ret;
		const auto pszBuffer = GetCStr();
		const auto uLength = GetLength();
		const auto uUniLength = (std::size_t)::MultiByteToWideChar(CP_UTF8, 0, pszBuffer, (int)uLength, nullptr, 0);
		ret.Resize(uUniLength);
		::MultiByteToWideChar(CP_UTF8, 0, pszBuffer, (int)uLength, ret.GetData(), (int)uUniLength);
		return std::move(ret);
	}
	template<>
	void UTF8String::xDisunify(UNIFIED_CHAR_SEQ &&ucsUnified){
		const auto uDisuniLength = (std::size_t)::WideCharToMultiByte(CP_UTF8, 0, ucsUnified.GetData(), (int)ucsUnified.GetSize(), nullptr, 0, nullptr, nullptr);
		Reserve(uDisuniLength + 1);
		const auto pszBuffer = GetCStr();
		::WideCharToMultiByte(CP_UTF8, 0, ucsUnified.GetData(), (int)ucsUnified.GetSize(), pszBuffer, (int)uDisuniLength, nullptr, nullptr);
		pszBuffer[uDisuniLength] = 0;
	}

	template<>
	UNIFIED_CHAR_SEQ ANSIString::xUnify() const {
		UNIFIED_CHAR_SEQ ret;
		const auto pszBuffer = GetCStr();
		const auto uLength = GetLength();
		const auto uUniLength = (std::size_t)::MultiByteToWideChar(CP_ACP, 0, pszBuffer, (int)uLength, nullptr, 0);
		ret.Resize(uUniLength);
		::MultiByteToWideChar(CP_ACP, 0, pszBuffer, (int)uLength, ret.GetData(), (int)uUniLength);
		return std::move(ret);
	}
	template<>
	void ANSIString::xDisunify(UNIFIED_CHAR_SEQ &&ucsUnified){
		const auto uDisuniLength = (std::size_t)::WideCharToMultiByte(CP_ACP, 0, ucsUnified.GetData(), (int)ucsUnified.GetSize(), nullptr, 0, nullptr, nullptr);
		Reserve(uDisuniLength + 1);
		const auto pszBuffer = GetCStr();
		::WideCharToMultiByte(CP_ACP, 0, ucsUnified.GetData(), (int)ucsUnified.GetSize(), pszBuffer, (int)uDisuniLength, nullptr, nullptr);
		pszBuffer[uDisuniLength] = 0;
	}

	template<>
	UNIFIED_CHAR_SEQ UTF16String::xUnify() const {
		const auto pszBuffer = GetCStr();
		return UNIFIED_CHAR_SEQ(pszBuffer, GetLength());
	}
	template<>
	void UTF16String::xDisunify(UNIFIED_CHAR_SEQ &&ucsUnified){
		Assign(ucsUnified.GetData(), ucsUnified.GetSize());
	}
}
