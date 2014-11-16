// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_EXCEPTION_HPP_
#define MCF_CORE_EXCEPTION_HPP_

#include <memory>
#include <cstddef>
#include "StringObserver.hpp"
#include "../Utilities/Assert.hpp"

namespace MCF {

template<typename CharT>
std::shared_ptr<CharT> CreateSharedString(const CharT *pchSrc, std::size_t uLen, std::size_t uAdditional = 0);

extern template std::shared_ptr<char> CreateSharedString<char>(const char *, std::size_t, std::size_t);
extern template std::shared_ptr<wchar_t> CreateSharedString<wchar_t>(const wchar_t *, std::size_t, std::size_t);
extern template std::shared_ptr<char16_t> CreateSharedString<char16_t>(const char16_t *, std::size_t, std::size_t);
extern template std::shared_ptr<char32_t> CreateSharedString<char32_t>(const char32_t *, std::size_t, std::size_t);

template<StringTypes TYPE_T>
auto CreateSharedString(const StringObserver<TYPE_T> &obsSrc, std::size_t uAdditional = 0){
	return CreateSharedString(obsSrc.GetBegin(), obsSrc.GetSize(), uAdditional);
}

template<typename CharT>
std::size_t GetSharedStringCapacity(const std::shared_ptr<CharT> &pchStr) noexcept {
	return reinterpret_cast<const std::size_t *>(pchStr.get())[-2];
}

template<typename CharT>
std::size_t GetSharedStringLength(const std::shared_ptr<CharT> &pchStr) noexcept {
	return reinterpret_cast<const std::size_t *>(pchStr.get())[-1];
}
template<typename CharT>
void SetSharedStringLength(const std::shared_ptr<CharT> &pchStr, std::size_t uNewLength) noexcept {
	ASSERT(GetSharedStringCapacity(pchStr) >= uNewLength);
	reinterpret_cast<std::size_t *>(pchStr.get())[-1] = uNewLength;
}

}

#endif
