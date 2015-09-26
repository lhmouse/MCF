// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_OBSERVER_HPP_
#define MCF_CORE_STRING_OBSERVER_HPP_

#include "../../MCFCRT/ext/alloca.h"
#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include "../Utilities/Defer.hpp"
#include "../Utilities/RationalFunctors.hpp"
#include "Exception.hpp"
#include <utility>
#include <iterator>
#include <type_traits>
#include <initializer_list>
#include <cstddef>

namespace MCF {

enum class StringType {
	NARROW,
	WIDE,
	UTF8,
	UTF16,
	UTF32,
	CESU8,
	ANSI,
};

template<StringType>
struct StringEncodingTrait;

template<>
struct StringEncodingTrait<StringType::NARROW> {
	using Char = char;
};
template<>
struct StringEncodingTrait<StringType::WIDE> {
	using Char = wchar_t;
};

template<>
struct StringEncodingTrait<StringType::UTF8> {
	using Char = char;
};
template<>
struct StringEncodingTrait<StringType::UTF16> {
	using Char = char16_t;
};
template<>
struct StringEncodingTrait<StringType::UTF32> {
	using Char = char32_t;
};
template<>
struct StringEncodingTrait<StringType::CESU8> {
	using Char = char;
};
template<>
struct StringEncodingTrait<StringType::ANSI> {
	using Char = char;
};

namespace Impl_StringObserver {
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

		const auto chFirst = *itToFindBegin;
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
				for(;;){
					if(itToFindBegin[nPos] == itToFindBegin[nCand]){
						++nCand;
						pTable[nPos] = nCand;
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

				if(itToFindBegin[1] == chFirst){
					nPos = 1;
					for(;;){
						if(itToFindBegin[nPos + 1] != chFirst){
							break;
						}
						pTable[nPos] = 0;

						++nPos;
						if(static_cast<std::size_t>(nPos) >= uFindCount - 1){
							break;
						}
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
				if(*itCur == chFirst){
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
			const auto nFallback = pTable[nMatchLen - 1];
			itCur += nMatchLen - nFallback;
			if(nFallback != 0){
				nMatchLen = nFallback;
				goto jFallback;
			}
		}
	}
}

template<StringType kTypeT>
class StringObserver {
public:
	using Char = typename StringEncodingTrait<kTypeT>::Char;

	enum : std::size_t {
		kNpos = Impl_StringObserver::kNpos
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
	constexpr StringObserver() noexcept
		: x_pchBegin(nullptr), x_pchEnd(nullptr)
	{
	}
	constexpr StringObserver(const Char *pchBegin, const Char *pchEnd) noexcept
		: x_pchBegin(pchBegin), x_pchEnd(pchEnd)
	{
	}
	constexpr StringObserver(std::nullptr_t, std::nullptr_t = nullptr) noexcept
		: StringObserver()
	{
	}
	constexpr StringObserver(const Char *pchBegin, std::size_t uLen) noexcept
		: x_pchBegin(pchBegin), x_pchEnd(pchBegin + uLen)
	{
	}
	constexpr StringObserver(std::initializer_list<Char> rhs) noexcept
		: StringObserver(rhs.begin(), rhs.size())
	{
	}
	explicit StringObserver(const Char *pszBegin) noexcept
		: StringObserver(pszBegin, Impl_StringObserver::StrEndOf(pszBegin))
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
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
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

	void Swap(StringObserver &rhs) noexcept {
		using std::swap;
		swap(x_pchBegin, rhs.x_pchBegin);
		swap(x_pchEnd,   rhs.x_pchEnd);
	}

	int Compare(const StringObserver &rhs) const noexcept {
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
		Assign(pszBegin, Impl_StringObserver::StrEndOf(pszBegin));
	}

	// 举例：
	//   Slice( 1,  5)   返回 "bcde"_rcs；
	//   Slice( 1, -5)   返回 "bc"；
	//   Slice( 5, -1)   返回 "fg"；
	//   Slice(-5, -1)   返回 "defg"。
	StringObserver Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd) const noexcept {
		const auto uLength = GetLength();
		return StringObserver(x_pchBegin + X_TranslateOffset(nBegin, uLength), x_pchBegin + X_TranslateOffset(nEnd, uLength));
	}

	// 举例：
	//   Find("def", 3)             返回 3；
	//   Find("def", 4)             返回 kNpos；
	//   FindBackward("def", 5)     返回 kNpos；
	//   FindBackward("def", 6)     返回 3。
	std::size_t Find(const StringObserver &obsToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = X_TranslateOffset(nBegin, uLength);
		const auto uLenToFind = obsToFind.GetLength();
		if(uLenToFind == 0){
			return uRealBegin;
		}
		if(uLength < uLenToFind){
			return kNpos;
		}
		if(uRealBegin + uLenToFind > uLength){
			return kNpos;
		}
		const auto uPos = Impl_StringObserver::StrStr(GetBegin() + uRealBegin, GetEnd(), obsToFind.GetBegin(), obsToFind.GetEnd());
		if(uPos == kNpos){
			return kNpos;
		}
		return uPos + uRealBegin;
	}
	std::size_t FindBackward(const StringObserver &obsToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = X_TranslateOffset(nEnd, uLength);
		const auto uLenToFind = obsToFind.GetLength();
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
			itToFindBegin(obsToFind.GetEnd()), itToFindEnd(obsToFind.GetBegin());
		const auto uPos = Impl_StringObserver::StrStr(itBegin, itEnd, itToFindBegin, itToFindEnd);
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
		const auto uPos = Impl_StringObserver::StrChrRep(GetBegin() + uRealBegin, GetEnd(), chToFind, uFindCount);
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
		const auto uPos = Impl_StringObserver::StrChrRep(itBegin, itEnd, chToFind, uFindCount);
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

	bool DoesOverlapWith(const StringObserver &rhs) const noexcept {
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
bool operator==(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	if(lhs.GetSize() != rhs.GetSize()){
		return false;
	}
	return lhs.Compare(rhs) == 0;
}
template<StringType kTypeT>
bool operator!=(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	if(lhs.GetSize() != rhs.GetSize()){
		return true;
	}
	return lhs.Compare(rhs) != 0;
}
template<StringType kTypeT>
bool operator<(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) < 0;
}
template<StringType kTypeT>
bool operator>(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) > 0;
}
template<StringType kTypeT>
bool operator<=(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) <= 0;
}
template<StringType kTypeT>
bool operator>=(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) >= 0;
}

template<StringType kTypeT>
void swap(StringObserver<kTypeT> &lhs, StringObserver<kTypeT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<StringType kTypeT>
decltype(auto) begin(const StringObserver<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
decltype(auto) cbegin(const StringObserver<kTypeT> &rhs) noexcept {
	return begin(rhs);
}
template<StringType kTypeT>
decltype(auto) end(const StringObserver<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringType kTypeT>
decltype(auto) cend(const StringObserver<kTypeT> &rhs) noexcept {
	return end(rhs);
}

extern template class StringObserver<StringType::NARROW>;
extern template class StringObserver<StringType::WIDE>;
extern template class StringObserver<StringType::UTF8>;
extern template class StringObserver<StringType::UTF16>;
extern template class StringObserver<StringType::UTF32>;
extern template class StringObserver<StringType::CESU8>;
extern template class StringObserver<StringType::ANSI>;

using NarrowStringObserver = StringObserver<StringType::NARROW>;
using WideStringObserver   = StringObserver<StringType::WIDE>;
using Utf8StringObserver   = StringObserver<StringType::UTF8>;
using Utf16StringObserver  = StringObserver<StringType::UTF16>;
using Utf32StringObserver  = StringObserver<StringType::UTF32>;
using Cesu8StringObserver  = StringObserver<StringType::CESU8>;
using AnsiStringObserver   = StringObserver<StringType::ANSI>;

// 字面量运算符。
// 注意 StringObserver 并不是所谓“零结尾的字符串”。
// 这些运算符经过特意设计防止这种用法。
template<typename CharT, CharT ...kCharsT>
[[deprecated("Be warned that encodings of narrow string literals vary from compilers to compilers and might even depend on encodings of source files on g++.")]]
extern inline auto operator""_nso() noexcept {
	static constexpr char s_achData[] = { kCharsT..., '$' };
	return NarrowStringObserver(s_achData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_wso() noexcept {
	static constexpr wchar_t s_awcData[] = { kCharsT..., '$' };
	return WideStringObserver(s_awcData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_u8so() noexcept {
	static constexpr char s_au8cData[] = { kCharsT..., '$' };
	return Utf8StringObserver(s_au8cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_u16so() noexcept {
	static constexpr char16_t s_au16cData[] = { kCharsT..., '$' };
	return Utf16StringObserver(s_au16cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_u32so() noexcept {
	static constexpr char32_t s_au32cData[] = { kCharsT..., '$' };
	return Utf32StringObserver(s_au32cData, sizeof...(kCharsT));
}

}

#endif
