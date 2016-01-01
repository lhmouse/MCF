// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_VIEW_HPP_
#define MCF_CORE_STRING_VIEW_HPP_

#include "../../MCFCRT/ext/alloca.h"
#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include "../Utilities/Defer.hpp"
#include "../Function/Comparators.hpp"
#include "Exception.hpp"
#include <utility>
#include <iterator>
#include <type_traits>
#include <initializer_list>
#include <cstddef>

namespace MCF {

enum class StringType {
	kNarrow,
	kWide,
	kUtf8,
	kUtf16,
	kUtf32,
	kCesu8,
	kAnsi,
};

template<StringType>
struct StringEncodingTrait;

template<>
struct StringEncodingTrait<StringType::kNarrow> {
	using Char = char;

	enum {
		kPrefersConversionViaUtf16 = 0,
		kPrefersConversionViaUtf32 = 0,
	};
};
template<>
struct StringEncodingTrait<StringType::kWide> {
	using Char = wchar_t;

	enum {
		kPrefersConversionViaUtf16 = 1,
		kPrefersConversionViaUtf32 = 0,
	};
};

template<>
struct StringEncodingTrait<StringType::kUtf8> {
	using Char = char;

	enum {
		kPrefersConversionViaUtf16 = 0,
		kPrefersConversionViaUtf32 = 0,
	};
};
template<>
struct StringEncodingTrait<StringType::kUtf16> {
	using Char = char16_t;

	enum {
		kPrefersConversionViaUtf16 = 1,
		kPrefersConversionViaUtf32 = 0,
	};
};
template<>
struct StringEncodingTrait<StringType::kUtf32> {
	using Char = char32_t;

	enum {
		kPrefersConversionViaUtf16 = 0,
		kPrefersConversionViaUtf32 = 1,
	};
};
template<>
struct StringEncodingTrait<StringType::kCesu8> {
	using Char = char;

	enum {
		kPrefersConversionViaUtf16 = 0,
		kPrefersConversionViaUtf32 = 0,
	};
};
template<>
struct StringEncodingTrait<StringType::kAnsi> {
	using Char = char;

	enum {
		kPrefersConversionViaUtf16 = 1,
		kPrefersConversionViaUtf32 = 0,
	};
};

namespace Impl_StringView {
	enum : std::size_t {
		kNpos = static_cast<std::size_t>(-1)
	};

	template<typename CharT>
	const CharT *StrEndOf(const CharT *pszBegin) noexcept {
		ASSERT(pszBegin);

		auto pchEnd = pszBegin;
		while(*pchEnd != CharT()){
			++pchEnd;
		}
		return pchEnd;
	}

	template<typename CharT, typename IteratorT>
	std::size_t StrChrRep(IteratorT itBegin, std::common_type_t<IteratorT> itEnd, CharT chToFind, std::size_t uFindCount) noexcept {
		ASSERT(uFindCount != 0);
		ASSERT(static_cast<std::size_t>(itEnd - itBegin) >= uFindCount);

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
		ASSERT(itToFindEnd != itToFindBegin);
		ASSERT(static_cast<std::size_t>(itEnd - itBegin) >= static_cast<std::size_t>(itToFindEnd - itToFindBegin));

		const auto uFindCount = static_cast<std::size_t>(itToFindEnd - itToFindBegin);

		std::ptrdiff_t *pTable;
		bool bTableAllocatedFromHeap;
		const auto uTableSize = uFindCount - 1;
		if(uTableSize >= 0x10000 / sizeof(std::ptrdiff_t)){
			pTable = ::new(std::nothrow) std::ptrdiff_t[uTableSize];
			bTableAllocatedFromHeap = true;
		} else {
			pTable = static_cast<std::ptrdiff_t *>(ALLOCA(uTableSize * sizeof(std::ptrdiff_t)));
			bTableAllocatedFromHeap = false;
		}
		DEFER([&]{ if(bTableAllocatedFromHeap){ ::delete[](pTable); }; });

		if(pTable){
			pTable[0] = 0;

			if(uFindCount > 2){
				std::ptrdiff_t nPos = 1, nCand = 0;
				do {
				jTryFallback:
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
						goto jTryFallback;
					}
					++nPos;
				} while(static_cast<std::size_t>(nPos) < uFindCount - 1);
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
		kNpos = Impl_StringView::kNpos
	};

	static_assert(std::is_integral<Char>::value, "Char must be an integral type.");

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
		ASSERT(uRet <= uLength);
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
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "StringView: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	const Char &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < GetSize());

		return GetBegin()[uIndex];
	}

	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		x_pchEnd = x_pchBegin;
	}

	void Swap(StringView &rhs) noexcept {
		using std::swap;
		swap(x_pchBegin, rhs.x_pchBegin);
		swap(x_pchEnd,   rhs.x_pchEnd);
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
		return Less()(x_pchBegin, rhs.x_pchEnd) && Less()(rhs.x_pchBegin, x_pchEnd);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	const Char &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
};

template<StringType kTypeT>
bool operator==(const StringView<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	if(lhs.GetSize() != rhs.GetSize()){
		return false;
	}
	return lhs.Compare(rhs) == 0;
}
template<StringType kTypeT>
bool operator!=(const StringView<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	if(lhs.GetSize() != rhs.GetSize()){
		return true;
	}
	return lhs.Compare(rhs) != 0;
}
template<StringType kTypeT>
bool operator<(const StringView<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) < 0;
}
template<StringType kTypeT>
bool operator>(const StringView<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) > 0;
}
template<StringType kTypeT>
bool operator<=(const StringView<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) <= 0;
}
template<StringType kTypeT>
bool operator>=(const StringView<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) >= 0;
}

template<StringType kTypeT>
void swap(StringView<kTypeT> &lhs, StringView<kTypeT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<StringType kTypeT>
decltype(auto) begin(const StringView<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
decltype(auto) cbegin(const StringView<kTypeT> &rhs) noexcept {
	return begin(rhs);
}
template<StringType kTypeT>
decltype(auto) end(const StringView<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringType kTypeT>
decltype(auto) cend(const StringView<kTypeT> &rhs) noexcept {
	return end(rhs);
}

extern template class StringView<StringType::kNarrow>;
extern template class StringView<StringType::kWide>;
extern template class StringView<StringType::kUtf8>;
extern template class StringView<StringType::kUtf16>;
extern template class StringView<StringType::kUtf32>;
extern template class StringView<StringType::kCesu8>;
extern template class StringView<StringType::kAnsi>;

using NarrowStringView = StringView<StringType::kNarrow>;
using WideStringView   = StringView<StringType::kWide>;
using Utf8StringView   = StringView<StringType::kUtf8>;
using Utf16StringView  = StringView<StringType::kUtf16>;
using Utf32StringView  = StringView<StringType::kUtf32>;
using Cesu8StringView  = StringView<StringType::kCesu8>;
using AnsiStringView   = StringView<StringType::kAnsi>;

// 字面量运算符。
// 注意 StringView 并不是所谓“零结尾的字符串”。
// 这些运算符经过特意设计防止这种用法。
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

}

#endif
