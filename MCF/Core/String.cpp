// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"

namespace MCF {

template class GenericString<char,		StringEncoding::UTF8>;
template class GenericString<char,		StringEncoding::ANSI>;
template class GenericString<wchar_t,	StringEncoding::UTF16>;

template<>
UnifiedString &UTF8String::xUnify(UnifiedString &ucsTemp) const {
	const auto pszSource = GetCStr();
	const auto nLength = (int)GetLength();
	const auto nUnifiedLength = ::MultiByteToWideChar(CP_UTF8, 0, pszSource, nLength, nullptr, 0);
	ucsTemp.Resize(::MultiByteToWideChar(CP_UTF8, 0, pszSource, nLength, ucsTemp.Reserve(nUnifiedLength), nUnifiedLength));
	return ucsTemp;
}
template<>
void UTF8String::xDisunify(const UnifiedString &ucsTemp){
	const auto pszSource = ucsTemp.GetCStr();
	const auto nLength = (int)ucsTemp.GetLength();
	const auto nDisunifiedLength = ::WideCharToMultiByte(CP_UTF8, 0, pszSource, nLength, nullptr, 0, nullptr, nullptr);
	Resize(::WideCharToMultiByte(CP_UTF8, 0, pszSource, nLength, Reserve(nDisunifiedLength), nDisunifiedLength, nullptr, nullptr));
}
template<>
void UTF8String::xDisunify(UnifiedString &&ucsTemp){
	xDisunify(ucsTemp);
}

template<>
UnifiedString &ANSIString::xUnify(UnifiedString &ucsTemp) const {
	const auto pszSource = GetCStr();
	const auto nLength = (int)GetLength();
	const auto nUnifiedLength = ::MultiByteToWideChar(CP_ACP, 0, pszSource, nLength, nullptr, 0);
	ucsTemp.Resize(::MultiByteToWideChar(CP_ACP, 0, pszSource, nLength, ucsTemp.Reserve(nUnifiedLength), nUnifiedLength));
	return ucsTemp;
}
template<>
void ANSIString::xDisunify(const UnifiedString &ucsTemp){
	const auto pszSource = ucsTemp.GetCStr();
	const auto nLength = (int)ucsTemp.GetLength();
	const auto nDisunifiedLength = ::WideCharToMultiByte(CP_ACP, 0, pszSource, nLength, nullptr, 0, nullptr, nullptr);
	Resize(::WideCharToMultiByte(CP_ACP, 0, pszSource, nLength, Reserve(nDisunifiedLength), nDisunifiedLength, nullptr, nullptr));
}
template<>
void ANSIString::xDisunify(UnifiedString &&ucsTemp){
	xDisunify(ucsTemp);
}

template<>
UnifiedString &UnifiedString::xUnify(UnifiedString & /* ucsTemp */) const {
	return *(UnifiedString *)this;
}
template<>
void UnifiedString::xDisunify(const UnifiedString &ucsTemp){
	Assign(ucsTemp);
}
template<>
void UnifiedString::xDisunify(UnifiedString &&ucsTemp){
	Assign(std::move(ucsTemp));
}

}
