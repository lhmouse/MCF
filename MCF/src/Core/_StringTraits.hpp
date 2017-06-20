// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_TRAITS_HPP_
#define MCF_CORE_STRING_TRAITS_HPP_

#include <type_traits>
#include <cstddef>
#include <climits>

namespace MCF {

namespace Impl_StringTraits {
	//-----------------------------------------------------------------------------
	// Types and encodings
	//-----------------------------------------------------------------------------

	enum class Type {
		kUtf8          =  0,
		kUtf16         =  1,
		kUtf32         =  2,
		kCesu8         =  3,
		kAnsi          =  4,
		kModifiedUtf8  =  5,
		kNarrow        = 98,
		kWide          = 99,
	};

	template<Type>
	struct Encoding;

	template<>
	struct Encoding<Type::kUtf8> {
		enum { kConversionPreference = 0 };
		using Char = char;
	};

	template<>
	struct Encoding<Type::kUtf16> {
		enum { kConversionPreference = -1 }; // UTF-16
		using Char = char16_t;
	};

	template<>
	struct Encoding<Type::kUtf32> {
		enum { kConversionPreference = 1 }; // UTF-32
		using Char = char32_t;
	};

	template<>
	struct Encoding<Type::kCesu8> {
		enum { kConversionPreference = 0 };
		using Char = char;
	};

	template<>
	struct Encoding<Type::kAnsi> {
		enum { kConversionPreference = -1 }; // UTF-16
		using Char = char;
	};

	template<>
	struct Encoding<Type::kModifiedUtf8> {
		enum { kConversionPreference = 0 };
		using Char = char;
	};

	template<>
	struct Encoding<Type::kNarrow> {
		enum { kConversionPreference = 0 };
		using Char = char;
	};

	template<>
	struct Encoding<Type::kWide> {
		enum { kConversionPreference = -1 }; // UTF-16
		using Char = wchar_t;
	};

	//-----------------------------------------------------------------------------
	// Generic algorithms
	//-----------------------------------------------------------------------------

	template<typename SelfBeginT>
	SelfBeginT Define(SelfBeginT itSelfBegin){
		std::ptrdiff_t nOffset = 0;
		for(;;){
			const auto chSelf = itSelfBegin[nOffset];
			if(chSelf == decltype(chSelf)()){
				return itSelfBegin + nOffset;
			}
			++nOffset;
		}
	}

	template<typename SelfBeginT, typename SelfEndT, typename ComparandBeginT, typename ComparandEndT>
	int Compare(SelfBeginT itSelfBegin, SelfEndT itSelfEnd, ComparandBeginT itComparandBegin, ComparandEndT itComparandEnd){
		std::ptrdiff_t nOffset = 0;
		for(;;){
			const bool bEndMarkSelf = itSelfBegin + nOffset == itSelfEnd;
			const bool bEndMarkComparand = itComparandBegin + nOffset == itComparandEnd;
			if(bEndMarkSelf || bEndMarkComparand){
				return bEndMarkSelf - bEndMarkComparand;
			}
			const auto chSelf = itSelfBegin[nOffset];
			const auto chComparand = itComparandBegin[nOffset];
			if(chSelf != chComparand){
				return (static_cast<std::make_unsigned_t<decltype(chSelf)>>(chSelf) < static_cast<std::make_unsigned_t<decltype(chComparand)>>(chComparand)) ? -1 : 1;
			}
			++nOffset;
		}
	}

	template<typename SelfBeginT, typename SelfEndT, typename ComparandT>
	SelfBeginT FindRepeat(SelfBeginT itSelfBegin, SelfEndT itSelfEnd, const ComparandT &chComparand, std::size_t uComparandCount){
		const auto nComparandCount = static_cast<std::ptrdiff_t>(uComparandCount);
		if(nComparandCount < 0){
			return itSelfEnd;
		}
		if(nComparandCount == 0){
			return itSelfBegin;
		}
		const auto nSelfCount = static_cast<std::ptrdiff_t>(itSelfEnd - itSelfBegin);
		if(nSelfCount < nComparandCount){
			return itSelfEnd;
		}

		std::ptrdiff_t nOffset = 0;
		for(;;){
			if(nSelfCount - nOffset < nComparandCount){
				return itSelfEnd;
			}
			std::ptrdiff_t nTestIndex = nComparandCount - 1;
			for(;;){
				const auto chSelf = itSelfBegin[nOffset + nTestIndex];
				if(chSelf != chComparand){
					nOffset += nTestIndex + 1;
					break;
				}
				if(nTestIndex == 0){
					return itSelfBegin + nOffset;
				}
				--nTestIndex;
			}
		}
	}

	template<typename SelfBeginT, typename SelfEndT, typename ComparandBeginT, typename ComparandEndT>
	SelfBeginT FindSpan(SelfBeginT itSelfBegin, SelfEndT itSelfEnd, ComparandBeginT itComparandBegin, ComparandEndT itComparandEnd){
		const auto nComparandCount = static_cast<std::ptrdiff_t>(itComparandEnd - itComparandBegin);
		if(nComparandCount < 0){
			return itSelfEnd;
		}
		if(nComparandCount == 0){
			return itSelfBegin;
		}
		const auto nSelfCount = static_cast<std::ptrdiff_t>(itSelfEnd - itSelfBegin);
		if(nSelfCount < nComparandCount){
			return itSelfEnd;
		}

		// https://en.wikipedia.org/wiki/Boyer-Moore-Horspool_algorithm
		//   table[pattern[i]] = pattern_count - (i + 1)    # where 0 <= i < pattern_count - 1
		// We store the offsets as small integers using saturation arithmetic for space efficiency. Bits that do not fit into a byte are truncated.
		__attribute__((__aligned__(64))) unsigned short aushBadCharacterTable[256];
		for(unsigned uIndex = 0; uIndex < 256; ++uIndex){
			aushBadCharacterTable[uIndex] = static_cast<unsigned short>((nComparandCount <= USHRT_MAX) ? nComparandCount : USHRT_MAX);
		}
		for(std::ptrdiff_t nGoodCharNext = ((nComparandCount <= USHRT_MAX) ? 0 : (nComparandCount - USHRT_MAX)) + 1; nGoodCharNext < nComparandCount; ++nGoodCharNext){
			const auto chGoodChar = itComparandBegin[nGoodCharNext - 1];
			aushBadCharacterTable[static_cast<unsigned char>(chGoodChar)] = static_cast<unsigned short>(nComparandCount - nGoodCharNext);
		}

		std::ptrdiff_t nOffset = 0;
		for(;;){
			if(nSelfCount - nOffset < nComparandCount){
				return itSelfEnd;
			}
			std::ptrdiff_t nTestIndex = nComparandCount - 1;
			const auto chLast = itSelfBegin[nOffset + nTestIndex];
			for(;;){
				const auto chSelf = itSelfBegin[nOffset + nTestIndex];
				const auto chComparand = itComparandBegin[nTestIndex];
				if(chSelf != chComparand){
					nOffset += static_cast<std::ptrdiff_t>(aushBadCharacterTable[static_cast<unsigned char>(chLast)]);
					break;
				}
				if(nTestIndex == 0){
					return itSelfBegin + nOffset;
				}
				--nTestIndex;
			}
		}
	}
}

}

#endif
