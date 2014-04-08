// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "String.hpp"

namespace MCF {

template<>
const UnifiedString *Utf8String::xUnify(UnifiedString &ucsTemp) const {
	const auto pszSource = GetCStr();
	const auto nLength = (int)GetLength();
	auto nUnifiedLength = ::MultiByteToWideChar(CP_UTF8, 0, pszSource, nLength, nullptr, 0);
	ucsTemp.Resize(nUnifiedLength);
	nUnifiedLength = ::MultiByteToWideChar(CP_UTF8, 0, pszSource, nLength, ucsTemp.GetCStr(), ucsTemp.GetCapacity());
	ucsTemp.Resize(nUnifiedLength);
	return nullptr;
}
template<>
void Utf8String::xDisunify(const UnifiedString &ucsTemp){
	const auto uOldLength = GetLength();
	const auto pszSource = ucsTemp.GetCStr();
	const auto nLength = (int)ucsTemp.GetLength();
	auto nDisunifiedLength = ::WideCharToMultiByte(CP_UTF8, 0, pszSource, nLength, nullptr, 0, nullptr, nullptr);
	Resize(uOldLength + nDisunifiedLength + 1);
	nDisunifiedLength = ::WideCharToMultiByte(CP_UTF8, 0, pszSource, nLength, GetCStr() + uOldLength, nDisunifiedLength, nullptr, nullptr);
	Resize(uOldLength + nDisunifiedLength);
}
template<>
void Utf8String::xDisunify(UnifiedString &&ucsTemp){
	xDisunify(ucsTemp);
}

template<>
const UnifiedString *AnsiString::xUnify(UnifiedString &ucsTemp) const {
	const auto pszSource = GetCStr();
	const auto nLength = (int)GetLength();
	auto nUnifiedLength = ::MultiByteToWideChar(CP_ACP, 0, pszSource, nLength, nullptr, 0);
	ucsTemp.Resize(nUnifiedLength);
	nUnifiedLength = ::MultiByteToWideChar(CP_ACP, 0, pszSource, nLength, ucsTemp.GetCStr(), ucsTemp.GetCapacity());
	ucsTemp.Resize(nUnifiedLength);
	return nullptr;
}
template<>
void AnsiString::xDisunify(const UnifiedString &ucsTemp){
	const auto uOldLength = GetLength();
	const auto pszSource = ucsTemp.GetCStr();
	const auto nLength = (int)ucsTemp.GetLength();
	auto nDisunifiedLength = ::WideCharToMultiByte(CP_ACP, 0, pszSource, nLength, nullptr, 0, nullptr, nullptr);
	Resize(uOldLength + nDisunifiedLength + 1);
	nDisunifiedLength = ::WideCharToMultiByte(CP_ACP, 0, pszSource, nLength, GetCStr() + uOldLength, nDisunifiedLength, nullptr, nullptr);
	Resize(uOldLength + nDisunifiedLength);
}
template<>
void AnsiString::xDisunify(UnifiedString &&ucsTemp){
	xDisunify(ucsTemp);
}

template<>
const UnifiedString *UnifiedString::xUnify(UnifiedString & /* ucsTemp */) const {
	return this;
}
template<>
void UnifiedString::xDisunify(const UnifiedString &ucsTemp){
	Append(ucsTemp);
}
template<>
void UnifiedString::xDisunify(UnifiedString &&ucsTemp){
	Append(std::move(ucsTemp));
}

}
