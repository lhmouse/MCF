// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_VIEW_HPP_
#define MCF_CORE_STRING_VIEW_HPP_

#include "_StringTraits.hpp"
#include "_Enumerator.hpp"
#include "Assert.hpp"
#include "Exception.hpp"
#include <iterator>
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<Impl_StringTraits::Type kTypeT>
class StringView {
public:
	enum : std::size_t { kNpos = static_cast<std::size_t>(-1) };

	using Char  = typename Impl_StringTraits::Encoding<kTypeT>::Char;

	static_assert(std::is_integral<Char>::value, "Char must be an integral type.");

	// 容器需求。
	using Element         = const Char;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <StringView>;
	using Enumerator      = Impl_Enumerator::Enumerator      <StringView>;

private:
	// 为了方便理解，想象此处使用的是所谓“插入式光标”：
	// 字符串内容：    a   b   c   d   e   f   g
	// 正光标位置：  0   1   2   3   4   5   6   7
	// 负光标位置： -8  -7  -6  -5  -4  -3  -2  -1
	static std::size_t X_TranslateOffset(std::ptrdiff_t nOffset, std::size_t uLength) noexcept {
		auto uRet = static_cast<std::size_t>(nOffset);
		if(nOffset < 0){
			uRet += uLength + 1;
		}
		MCF_DEBUG_CHECK(uRet <= uLength);
		return uRet;
	}

private:
	const Char *x_pchBegin;
	const Char *x_pchEnd;

public:
	constexpr StringView() noexcept
		: x_pchBegin(nullptr), x_pchEnd(nullptr)
	{ }
	constexpr StringView(const Char *pchBegin, const Char *pchEnd) noexcept
		: x_pchBegin(pchBegin), x_pchEnd(pchEnd)
	{ }
	constexpr StringView(std::nullptr_t, std::nullptr_t = nullptr) noexcept
		: StringView()
	{ }
	constexpr StringView(const Char *pchBegin, std::size_t uLen) noexcept
		: StringView(pchBegin, pchBegin + uLen)
	{ }
	explicit StringView(const Char *pszBegin) noexcept
		: StringView(pszBegin, Impl_StringTraits::Define(pszBegin))
	{ }

public:
	// 容器需求。
	bool IsEmpty() const noexcept {
		return x_pchBegin == x_pchEnd;
	}
	void Clear() noexcept {
		x_pchBegin = nullptr;
		x_pchEnd   = nullptr;
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		try {
			const auto itBegin = GetBegin();
			const auto itEnd = GetEnd();
			for(auto itCur = itBegin; itCur != itEnd; ++itCur){
				*itOutput = *itCur;
				++itOutput;
			}
		} catch(...){
			Clear();
			throw;
		}
		Clear();
		return itOutput;
	}

	const Element *GetFirst() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	Element *GetFirst() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	Element *GetLast() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		return pBegin + uOffset - 1;
	}
	Element *GetPrev(const Element *pPos) noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		return pBegin + uOffset - 1;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset + 1 == GetSize()){
			return nullptr;
		}
		return pBegin + uOffset + 1;
	}
	Element *GetNext(const Element *pPos) noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset + 1 == GetSize()){
			return nullptr;
		}
		return pBegin + uOffset + 1;
	}

	ConstEnumerator EnumerateFirst() const noexcept {
		return ConstEnumerator(*this, GetFirst());
	}
	Enumerator EnumerateFirst() noexcept {
		return Enumerator(*this, GetFirst());
	}
	ConstEnumerator EnumerateConstFirst() const noexcept {
		return EnumerateFirst();
	}
	ConstEnumerator EnumerateLast() const noexcept {
		return ConstEnumerator(*this, GetLast());
	}
	Enumerator EnumerateLast() noexcept {
		return Enumerator(*this, GetLast());
	}
	ConstEnumerator EnumerateConstLast() const noexcept {
		return EnumerateLast();
	}
	constexpr ConstEnumerator EnumerateSingular() const noexcept {
		return ConstEnumerator(*this, nullptr);
	}
	Enumerator EnumerateSingular() noexcept {
		return Enumerator(*this, nullptr);
	}
	constexpr ConstEnumerator EnumerateConstSingular() const noexcept {
		return EnumerateSingular();
	}

	void Swap(StringView &svOther) noexcept {
		using std::swap;
		swap(x_pchBegin, svOther.x_pchBegin);
		swap(x_pchEnd,   svOther.x_pchEnd);
	}

	// StringView 需求。
	const Char *GetBegin() const noexcept {
		return x_pchBegin;
	}
	const Char *GetEnd() const noexcept {
		return x_pchEnd;
	}
	std::size_t GetSize() const noexcept {
		return static_cast<std::size_t>(x_pchEnd - x_pchBegin);
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	const Char &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"StringView: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	const Char &UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_DEBUG_CHECK(uIndex < GetSize());

		return GetBegin()[uIndex];
	}

	int Compare(const StringView &svOther) const noexcept {
		return Impl_StringTraits::Compare(GetBegin(), GetEnd(), svOther.GetBegin(), svOther.GetEnd());
	}

	void Assign(const Char *pchBegin, const Char *pchEnd) noexcept {
		x_pchBegin = pchBegin;
		x_pchEnd = pchEnd;
	}
	void Assign(std::nullptr_t, std::nullptr_t = nullptr) noexcept {
		x_pchBegin = nullptr;
		x_pchEnd = nullptr;
	}
	void Assign(const Char *pchBegin, std::size_t uLen) noexcept {
		Assign(pchBegin, pchBegin + uLen);
	}
	void Assign(const Char *pszBegin) noexcept {
		Assign(pszBegin, Impl_StringTraits::Define(pszBegin));
	}

	// 举例：
	//   Slice( 1,  5)   返回 "bcde"；
	//   Slice( 1, -5)   返回 "bc"；
	//   Slice( 5, -1)   返回 "fg"；
	//   Slice(-5, -1)   返回 "defg"。
	StringView Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd) const noexcept {
		const auto uRealBegin = X_TranslateOffset(nBegin, GetLength());
		const auto uRealEnd = X_TranslateOffset(nEnd, GetLength());
		return StringView(GetBegin() + uRealBegin, GetEnd() + uRealEnd);
	}

	// 举例：
	//   Find("def", 3)             返回 3；
	//   Find("def", 4)             返回 kNpos；
	//   FindBackward("def", 5)     返回 kNpos；
	//   FindBackward("def", 6)     返回 3。
	std::size_t Find(const StringView &svToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		const auto uRealBegin = X_TranslateOffset(nBegin, GetLength());
		const auto itRealBegin = GetBegin() + uRealBegin;
		const auto itRealEnd = GetEnd();
		const auto itPosition = Impl_StringTraits::FindSpan(itRealBegin, itRealEnd, svToFind.GetBegin(), svToFind.GetEnd());
		if(itPosition == itRealEnd){
			return kNpos;
		}
		return static_cast<std::size_t>(itPosition - itRealBegin);
	}
	std::size_t FindBackward(const StringView &svToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uRealEnd = X_TranslateOffset(nEnd, GetLength());
		const auto itRealBegin = std::make_reverse_iterator(GetBegin() + uRealEnd);
		const auto itRealEnd = std::make_reverse_iterator(GetBegin());
		const auto itPosition = Impl_StringTraits::FindSpan(itRealBegin, itRealEnd, std::make_reverse_iterator(svToFind.GetEnd()), std::make_reverse_iterator(svToFind.GetBegin()));
		if(itPosition == itRealEnd){
			return kNpos;
		}
		return static_cast<std::size_t>(itPosition - itRealBegin);
	}
	// 举例：
	//   Find('c', 3)           返回 kNpos；
	//   Find('d', 3)           返回 3；
	//   FindBackward('c', 3)   返回 2；
	//   FindBackward('d', 3)   返回 kNpos。
	std::size_t FindRep(Char chToFind, std::size_t uFindCount, std::ptrdiff_t nBegin = 0) const noexcept {
		const auto uRealBegin = X_TranslateOffset(nBegin, GetLength());
		const auto itRealBegin = GetBegin() + uRealBegin;
		const auto itRealEnd = GetEnd();
		const auto itPosition = Impl_StringTraits::FindRepeat(itRealBegin, itRealEnd, chToFind, uFindCount);
		if(itPosition == itRealEnd){
			return kNpos;
		}
		return static_cast<std::size_t>(itPosition - itRealBegin);
	}
	std::size_t FindRepBackward(Char chToFind, std::size_t uFindCount, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uRealEnd = X_TranslateOffset(nEnd, GetLength());
		const auto itRealBegin = std::make_reverse_iterator(GetBegin() + uRealEnd);
		const auto itRealEnd = std::make_reverse_iterator(GetBegin());
		const auto itPosition = Impl_StringTraits::FindRepeat(itRealBegin, itRealEnd, chToFind, uFindCount);
		if(itPosition == itRealEnd){
			return kNpos;
		}
		return static_cast<std::size_t>(itPosition - itRealBegin);
	}
	std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return FindRep(chToFind, 1, nBegin);
	}
	std::size_t FindBackward(Char chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return FindRepBackward(chToFind, 1, nEnd);
	}

	bool DoesOverlapWith(const StringView &svOther) const noexcept {
		return (x_pchBegin < svOther.x_pchEnd) && (svOther.x_pchBegin < x_pchEnd);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	const Char &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}

	bool operator==(const StringView &svOther) noexcept {
		if(GetSize() != svOther.GetSize()){
			return false;
		}
		return Compare(svOther) == 0;
	}
	bool operator!=(const StringView &svOther) noexcept {
		if(GetSize() != svOther.GetSize()){
			return true;
		}
		return Compare(svOther) != 0;
	}
	bool operator<(const StringView &svOther) noexcept {
		return Compare(svOther) < 0;
	}
	bool operator>(const StringView &svOther) noexcept {
		return Compare(svOther) > 0;
	}
	bool operator<=(const StringView &svOther) noexcept {
		return Compare(svOther) <= 0;
	}
	bool operator>=(const StringView &svOther) noexcept {
		return Compare(svOther) >= 0;
	}

	friend void swap(StringView &svSelf, StringView &svOther) noexcept {
		svSelf.Swap(svOther);
	}

	friend decltype(auto) begin(const StringView &svOther) noexcept {
		return svOther.GetBegin();
	}
	friend decltype(auto) cbegin(const StringView &svOther) noexcept {
		return begin(svOther);
	}
	friend decltype(auto) end(const StringView &svOther) noexcept {
		return svOther.GetEnd();
	}
	friend decltype(auto) cend(const StringView &svOther) noexcept {
		return end(svOther);
	}
};

extern template class StringView<Impl_StringTraits::Type::kUtf8>;
extern template class StringView<Impl_StringTraits::Type::kUtf16>;
extern template class StringView<Impl_StringTraits::Type::kUtf32>;
extern template class StringView<Impl_StringTraits::Type::kCesu8>;
extern template class StringView<Impl_StringTraits::Type::kAnsi>;
extern template class StringView<Impl_StringTraits::Type::kModifiedUtf8>;
extern template class StringView<Impl_StringTraits::Type::kNarrow>;
extern template class StringView<Impl_StringTraits::Type::kWide>;

using Utf8StringView         = StringView<Impl_StringTraits::Type::kUtf8>;
using Utf16StringView        = StringView<Impl_StringTraits::Type::kUtf16>;
using Utf32StringView        = StringView<Impl_StringTraits::Type::kUtf32>;
using Cesu8StringView        = StringView<Impl_StringTraits::Type::kCesu8>;
using AnsiStringView         = StringView<Impl_StringTraits::Type::kAnsi>;
using ModifiedUtf8StringView = StringView<Impl_StringTraits::Type::kModifiedUtf8>;
using NarrowStringView       = StringView<Impl_StringTraits::Type::kNarrow>;
using WideStringView         = StringView<Impl_StringTraits::Type::kWide>;

// 字面量运算符。
// 注意 StringView 并不是所谓“零结尾的字符串”。
// 这些运算符经过特意设计防止这种用法。
template<typename CharT, CharT ...kCharsT>
extern inline Utf8StringView operator""_u8sv() noexcept {
	static constexpr char s_au8cData[] = { kCharsT..., '$' };
	return Utf8StringView(s_au8cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline Utf16StringView operator""_u16sv() noexcept {
	static constexpr char16_t s_au16cData[] = { kCharsT..., '$' };
	return Utf16StringView(s_au16cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline Utf32StringView operator""_u32sv() noexcept {
	static constexpr char32_t s_au32cData[] = { kCharsT..., '$' };
	return Utf32StringView(s_au32cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline NarrowStringView operator""_nsv() noexcept {
	static constexpr char s_achData[] = { kCharsT..., '$' };
	return NarrowStringView(s_achData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline WideStringView operator""_wsv() noexcept {
	static constexpr wchar_t s_awcData[] = { kCharsT..., '$' };
	return WideStringView(s_awcData, sizeof...(kCharsT));
}

}

#endif
