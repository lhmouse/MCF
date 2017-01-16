// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_VIEW_HPP_
#define MCF_CORE_STRING_VIEW_HPP_

#include "_Enumerator.hpp"
#include "Assert.hpp"
#include "CountOf.hpp"
#include "Defer.hpp"
#include "Exception.hpp"
#include <MCFCRT/ext/alloca.h>
#include <iterator>
#include <utility>
#include <type_traits>
#include <initializer_list>
#include <cstddef>

namespace MCF {

enum class StringType {
	kUtf8          =  0,
	kUtf16         =  1,
	kUtf32         =  2,
	kCesu8         =  3,
	kAnsi          =  4,
	kModifiedUtf8  =  5,
	kNarrow        = 98,
	kWide          = 99,
};

template<StringType>
struct StringEncodingTrait;

template<>
struct StringEncodingTrait<StringType::kUtf8> {
	using Char = char;
	static constexpr int kConversionPreference = 0;
};
template<>
struct StringEncodingTrait<StringType::kUtf16> {
	using Char = char16_t;
	static constexpr int kConversionPreference = -1; // UTF-16
};
template<>
struct StringEncodingTrait<StringType::kUtf32> {
	using Char = char32_t;
	static constexpr int kConversionPreference = 1; // UTF-32
};
template<>
struct StringEncodingTrait<StringType::kCesu8> {
	using Char = char;
	static constexpr int kConversionPreference = 0;
};
template<>
struct StringEncodingTrait<StringType::kAnsi> {
	using Char = char;
	static constexpr int kConversionPreference = -1; // UTF-16
};
template<>
struct StringEncodingTrait<StringType::kModifiedUtf8> {
	using Char = char;
	static constexpr int kConversionPreference = 0;
};
template<>
struct StringEncodingTrait<StringType::kNarrow> {
	using Char = char;
	static constexpr int kConversionPreference = 0;
};
template<>
struct StringEncodingTrait<StringType::kWide> {
	using Char = wchar_t;
	static constexpr int kConversionPreference = -1; // UTF-16
};

namespace Impl_StringView {
	enum : std::size_t {
		kNpos = static_cast<std::size_t>(-1),
	};

	template<typename CharT>
	const CharT *StrEndOf(const CharT *pszBegin) noexcept {
		MCF_DEBUG_CHECK(pszBegin);

		auto pchEnd = pszBegin;
		while(*pchEnd != CharT()){
			++pchEnd;
		}
		return pchEnd;
	}

	template<typename CharT, typename IteratorT>
	std::size_t StrChrRep(IteratorT itBegin, std::common_type_t<IteratorT> itEnd, CharT chToFind, std::size_t uFindCount) noexcept {
		MCF_DEBUG_CHECK(uFindCount != 0);
		MCF_DEBUG_CHECK(static_cast<std::size_t>(itEnd - itBegin) >= uFindCount);

		auto itCur = itBegin;
		for(;;){
			for(;;){
				if(itEnd - itCur < static_cast<std::ptrdiff_t>(uFindCount)){
					return kNpos;
				}
				if(*itCur == chToFind){
					break;
				}
				++itCur;
			}

			std::ptrdiff_t nMatchLen = 1;
			for(;;){
				if(static_cast<std::size_t>(nMatchLen) >= uFindCount){
					return static_cast<std::size_t>(itCur - itBegin);
				}
				if(itCur[nMatchLen] != chToFind){
					break;
				}
				++nMatchLen;
			}
			itCur += nMatchLen;
			++itCur;
		}
	}

	template<typename IteratorT, typename ToFindIteratorT>
	std::size_t StrStr(IteratorT itBegin, std::common_type_t<IteratorT> itEnd, ToFindIteratorT itToFindBegin, std::common_type_t<ToFindIteratorT> itToFindEnd) noexcept {
		MCF_DEBUG_CHECK(itToFindEnd != itToFindBegin);
		MCF_DEBUG_CHECK(static_cast<std::size_t>(itEnd - itBegin) >= static_cast<std::size_t>(itToFindEnd - itToFindBegin));

		const auto uFindCount = static_cast<std::size_t>(itToFindEnd - itToFindBegin);

		std::ptrdiff_t *pTable;
		bool bTableWasAllocatedFromHeap;
		const auto uTableSize = uFindCount - 1;
		if(uTableSize >= 0x10000 / sizeof(std::ptrdiff_t)){
			pTable = ::new(std::nothrow) std::ptrdiff_t[uTableSize];
			bTableWasAllocatedFromHeap = true;
		} else {
			pTable = static_cast<std::ptrdiff_t *>(_MCFCRT_ALLOCA(uTableSize * sizeof(std::ptrdiff_t)));
			bTableWasAllocatedFromHeap = false;
		}
		const auto vFreeTable = Defer([&]{ if(bTableWasAllocatedFromHeap){ ::delete[](pTable); }; });

		if(pTable){
			pTable[0] = 0;

			if(uFindCount > 2){
				std::ptrdiff_t nPos = 1, nCand = 0;
				for(;;){
					if(itToFindBegin[nPos] == itToFindBegin[nCand]){
						++nCand;
						if(itToFindBegin[nPos + 1] == itToFindBegin[nCand]){
							pTable[nPos] = pTable[nCand - 1];
						} else {
							pTable[nPos] = nCand;
						}
					} else if(nCand == 0){
						pTable[nPos] = 0;
					} else {
						nCand = pTable[nCand - 1];
						continue;
					}
					++nPos;
					if(static_cast<std::size_t>(nPos) >= uFindCount - 1){
						break;
					}
				}
			}
		}

		auto itCur = itBegin;
		for(;;){
			for(;;){
				if(itEnd - itCur < static_cast<std::ptrdiff_t>(uFindCount)){
					return kNpos;
				}
				if(*itCur == *itToFindBegin){
					break;
				}
				++itCur;
			}

			std::ptrdiff_t nMatchLen = 1;
			for(;;){
				if(static_cast<std::size_t>(nMatchLen) >= uFindCount){
					return static_cast<std::size_t>(itCur - itBegin);
				}
		jFallback:
				if(itCur[nMatchLen] != itToFindBegin[nMatchLen]){
					break;
				}
				++nMatchLen;
			}
			if(!pTable){
				++itCur;
				continue;
			}
			itCur += nMatchLen;
			const auto nFallback = pTable[nMatchLen - 1];
			if(nFallback != 0){
				itCur -= nFallback;
				nMatchLen = nFallback;
				goto jFallback;
			}
		}
	}
}

template<StringType kTypeT>
class StringView {
public:
	using Char = typename StringEncodingTrait<kTypeT>::Char;

	enum : std::size_t {
		kNpos = Impl_StringView::kNpos,
	};

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

	// 以下均以此字符串为例。
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
	{
	}
	constexpr StringView(const Char *pchBegin, const Char *pchEnd) noexcept
		: x_pchBegin(pchBegin), x_pchEnd(pchEnd)
	{
	}
	constexpr StringView(std::nullptr_t, std::nullptr_t = nullptr) noexcept
		: StringView()
	{
	}
	constexpr StringView(const Char *pchBegin, std::size_t uLen) noexcept
		: StringView(pchBegin, pchBegin + uLen)
	{
	}
	constexpr StringView(std::initializer_list<Char> rhs) noexcept
		: StringView(rhs.begin(), rhs.size())
	{
	}
	explicit StringView(const Char *pszBegin) noexcept
		: StringView(pszBegin, Impl_StringView::StrEndOf(pszBegin))
	{
	}

public:
	// 容器需求。
	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		x_pchEnd = x_pchBegin;
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		try {
			const auto pBegin = GetBegin();
			const auto pEnd = GetEnd();
			for(auto p = pBegin; p != pEnd; ++p){
				*itOutput = *p;
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

	void Swap(StringView &rhs) noexcept {
		using std::swap;
		swap(x_pchBegin, rhs.x_pchBegin);
		swap(x_pchEnd,   rhs.x_pchEnd);
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

	int Compare(const StringView &rhs) const noexcept {
		using UChar = std::make_unsigned_t<Char>;

		auto pLRead = GetBegin();
		const auto pLEnd = GetEnd();
		auto pRRead = rhs.GetBegin();
		const auto pREnd = rhs.GetEnd();
		for(;;){
			const int nLAtEnd = (pLRead == pLEnd) ? 3 : 0;
			const int nRAtEnd = (pRRead == pREnd) ? 1 : 0;
			const int nResult = 2 - (nLAtEnd ^ nRAtEnd);
			if(nResult != 2){
				return nResult;
			}

			const auto uchLhs = static_cast<UChar>(*pLRead);
			const auto uchRhs = static_cast<UChar>(*pRRead);
			if(uchLhs != uchRhs){
				return (uchLhs < uchRhs) ? -1 : 1;
			}
			++pLRead;
			++pRRead;
		}
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
	void Assign(std::initializer_list<Char> rhs) noexcept {
		Assign(rhs.begin(), rhs.end());
	}
	void Assign(const Char *pszBegin) noexcept {
		Assign(pszBegin, Impl_StringView::StrEndOf(pszBegin));
	}

	// 举例：
	//   Slice( 1,  5)   返回 "bcde"；
	//   Slice( 1, -5)   返回 "bc"；
	//   Slice( 5, -1)   返回 "fg"；
	//   Slice(-5, -1)   返回 "defg"。
	StringView Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd) const noexcept {
		const auto uLength = GetLength();
		return StringView(x_pchBegin + X_TranslateOffset(nBegin, uLength), x_pchBegin + X_TranslateOffset(nEnd, uLength));
	}

	// 举例：
	//   Find("def", 3)             返回 3；
	//   Find("def", 4)             返回 kNpos；
	//   FindBackward("def", 5)     返回 kNpos；
	//   FindBackward("def", 6)     返回 3。
	std::size_t Find(const StringView &vToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = X_TranslateOffset(nBegin, uLength);
		const auto uLenToFind = vToFind.GetLength();
		if(uLenToFind == 0){
			return uRealBegin;
		}
		if(uLength < uLenToFind){
			return kNpos;
		}
		if(uRealBegin + uLenToFind > uLength){
			return kNpos;
		}
		const auto uPos = Impl_StringView::StrStr(GetBegin() + uRealBegin, GetEnd(), vToFind.GetBegin(), vToFind.GetEnd());
		if(uPos == kNpos){
			return kNpos;
		}
		return uPos + uRealBegin;
	}
	std::size_t FindBackward(const StringView &vToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = X_TranslateOffset(nEnd, uLength);
		const auto uLenToFind = vToFind.GetLength();
		if(uLenToFind == 0){
			return uRealEnd;
		}
		if(uLength < uLenToFind){
			return kNpos;
		}
		if(uRealEnd < uLenToFind){
			return kNpos;
		}
		std::reverse_iterator<const Char *> itBegin(GetBegin() + uRealEnd), itEnd(GetBegin()),
			itToFindBegin(vToFind.GetEnd()), itToFindEnd(vToFind.GetBegin());
		const auto uPos = Impl_StringView::StrStr(itBegin, itEnd, itToFindBegin, itToFindEnd);
		if(uPos == kNpos){
			return kNpos;
		}
		return uRealEnd - uPos - uLenToFind;
	}

	// 举例：
	//   Find('c', 3)           返回 kNpos；
	//   Find('d', 3)           返回 3；
	//   FindBackward('c', 3)   返回 2；
	//   FindBackward('d', 3)   返回 kNpos。
	std::size_t FindRep(Char chToFind, std::size_t uFindCount, std::ptrdiff_t nBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = X_TranslateOffset(nBegin, uLength);
		if(uFindCount == 0){
			return uRealBegin;
		}
		if(uLength < uFindCount){
			return kNpos;
		}
		if(uRealBegin + uFindCount > uLength){
			return kNpos;
		}
		const auto uPos = Impl_StringView::StrChrRep(GetBegin() + uRealBegin, GetEnd(), chToFind, uFindCount);
		if(uPos == kNpos){
			return kNpos;
		}
		return uPos + uRealBegin;
	}
	std::size_t FindRepBackward(Char chToFind, std::size_t uFindCount, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = X_TranslateOffset(nEnd, uLength);
		if(uFindCount == 0){
			return uRealEnd;
		}
		if(uLength < uFindCount){
			return kNpos;
		}
		if(uRealEnd < uFindCount){
			return kNpos;
		}
		std::reverse_iterator<const Char *> itBegin(GetBegin() + uRealEnd), itEnd(GetBegin());
		const auto uPos = Impl_StringView::StrChrRep(itBegin, itEnd, chToFind, uFindCount);
		if(uPos == kNpos){
			return kNpos;
		}
		return uRealEnd - uPos - uFindCount;
	}
	std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return FindRep(chToFind, 1, nBegin);
	}
	std::size_t FindBackward(Char chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return FindRepBackward(chToFind, 1, nEnd);
	}

	bool DoesOverlapWith(const StringView &rhs) const noexcept {
		return (x_pchBegin < rhs.x_pchEnd) && (rhs.x_pchBegin < x_pchEnd);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	const Char &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}

	bool operator==(const StringView &rhs) noexcept {
		if(GetSize() != rhs.GetSize()){
			return false;
		}
		return Compare(rhs) == 0;
	}
	bool operator!=(const StringView &rhs) noexcept {
		if(GetSize() != rhs.GetSize()){
			return true;
		}
		return Compare(rhs) != 0;
	}
	bool operator<(const StringView &rhs) noexcept {
		return Compare(rhs) < 0;
	}
	bool operator>(const StringView &rhs) noexcept {
		return Compare(rhs) > 0;
	}
	bool operator<=(const StringView &rhs) noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator>=(const StringView &rhs) noexcept {
		return Compare(rhs) >= 0;
	}

	friend void swap(StringView &lhs, StringView &rhs) noexcept {
		lhs.Swap(rhs);
	}

	friend decltype(auto) begin(const StringView &rhs) noexcept {
		return rhs.GetBegin();
	}
	friend decltype(auto) cbegin(const StringView &rhs) noexcept {
		return begin(rhs);
	}
	friend decltype(auto) end(const StringView &rhs) noexcept {
		return rhs.GetEnd();
	}
	friend decltype(auto) cend(const StringView &rhs) noexcept {
		return end(rhs);
	}
};

extern template class StringView<StringType::kUtf8>;
extern template class StringView<StringType::kUtf16>;
extern template class StringView<StringType::kUtf32>;
extern template class StringView<StringType::kCesu8>;
extern template class StringView<StringType::kAnsi>;
extern template class StringView<StringType::kModifiedUtf8>;
extern template class StringView<StringType::kNarrow>;
extern template class StringView<StringType::kWide>;

using Utf8StringView         = StringView<StringType::kUtf8>;
using Utf16StringView        = StringView<StringType::kUtf16>;
using Utf32StringView        = StringView<StringType::kUtf32>;
using Cesu8StringView        = StringView<StringType::kCesu8>;
using AnsiStringView         = StringView<StringType::kAnsi>;
using ModifiedUtf8StringView = StringView<StringType::kModifiedUtf8>;
using NarrowStringView       = StringView<StringType::kNarrow>;
using WideStringView         = StringView<StringType::kWide>;

// 字面量运算符。
// 注意 StringView 并不是所谓“零结尾的字符串”。
// 这些运算符经过特意设计防止这种用法。
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_u8sv() noexcept {
	static constexpr char s_au8cData[] = { kCharsT..., '$' };
	return Utf8StringView(s_au8cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_u16sv() noexcept {
	static constexpr char16_t s_au16cData[] = { kCharsT..., '$' };
	return Utf16StringView(s_au16cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_u32sv() noexcept {
	static constexpr char32_t s_au32cData[] = { kCharsT..., '$' };
	return Utf32StringView(s_au32cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_nsv() noexcept {
	static constexpr char s_achData[] = { kCharsT..., '$' };
	return NarrowStringView(s_achData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_wsv() noexcept {
	static constexpr wchar_t s_awcData[] = { kCharsT..., '$' };
	return WideStringView(s_awcData, sizeof...(kCharsT));
}

}

#endif
