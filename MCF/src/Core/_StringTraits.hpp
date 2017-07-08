// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_TRAITS_HPP_
#define MCF_CORE_STRING_TRAITS_HPP_

#include <MCFCRT/env/expect.h>
#include <type_traits>
#include <cstddef>

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

	template<typename SelfBeginT, typename SelfEndT, typename OtherBeginT, typename OtherEndT>
	int Compare(SelfBeginT itSelfBegin, SelfEndT itSelfEnd, OtherBeginT itOtherBegin, OtherEndT itOtherEnd){
		std::ptrdiff_t nOffset = 0;
		for(;;){
			const bool bEndMarkSelf = itSelfBegin + nOffset == itSelfEnd;
			const bool bEndMarkOther = itOtherBegin + nOffset == itOtherEnd;
			if(bEndMarkSelf || bEndMarkOther){
				return bEndMarkSelf - bEndMarkOther;
			}
			const auto chSelf = itSelfBegin[nOffset];
			const auto chOther = itOtherBegin[nOffset];
			if(chSelf != chOther){
				return (static_cast<std::make_unsigned_t<decltype(chSelf)>>(chSelf) < static_cast<std::make_unsigned_t<decltype(chOther)>>(chOther)) ? -1 : 1;
			}
			++nOffset;
		}
	}

	template<typename TextBeginT, typename TextEndT, typename PatternBeginT, typename PatternEndT>
	TextBeginT FindSpan(TextBeginT itTextBegin, TextEndT itTextEnd, PatternBeginT itPatternBegin, PatternEndT itPatternEnd){
		const auto nPatternLength = static_cast<std::ptrdiff_t>(itPatternEnd - itPatternBegin);
		if(nPatternLength < 0){
			return itTextEnd;
		}
		if(nPatternLength == 0){
			return itTextBegin;
		}
		const auto nTextCount = static_cast<std::ptrdiff_t>(itTextEnd - itTextBegin);
		if(nTextCount < nPatternLength){
			return itTextEnd;
		}

		// https://en.wikipedia.org/wiki/Boyer-Moore-Horspool_algorithm
		// We store the offsets as small integers using saturation arithmetic for space efficiency. Bits that do not fit into a byte are truncated.
		constexpr unsigned kBcrTableSize = 256;
		__attribute__((__aligned__(64))) short ashBcrTable[kBcrTableSize];
		const std::ptrdiff_t nMaxBcrShift = (nPatternLength <= 0x7FFF) ? nPatternLength : 0x7FFF;
		for(unsigned uIndex = 0; uIndex < kBcrTableSize; ++uIndex){
			ashBcrTable[uIndex] = static_cast<short>(nMaxBcrShift);
		}
		for(std::ptrdiff_t nBcrShift = nMaxBcrShift - 1; nBcrShift > 0; --nBcrShift){
			const auto chGoodChar = itPatternBegin[nPatternLength - nBcrShift - 1];
			ashBcrTable[static_cast<std::make_unsigned_t<decltype(chGoodChar)>>(chGoodChar) % kBcrTableSize] = static_cast<short>(nBcrShift);
		}

		// https://en.wikipedia.org/wiki/Boyer-Moore_string_search_algorithm
		// We create the GSR table from an intermediate table of suffix offsets.
		constexpr unsigned kGsrTableSize = 512;
		__attribute__((__aligned__(64))) short ashGsrOffsetTable[kGsrTableSize];
		const auto nGsrTableSize = (nPatternLength <= kGsrTableSize) ? nPatternLength : kGsrTableSize;
		std::ptrdiff_t nGsrCandidateLength = 0;
		ashGsrOffsetTable[0] = 1;
		for(std::ptrdiff_t nTestIndex = 1; nTestIndex < nGsrTableSize; ++nTestIndex){
			const auto chTest = itPatternBegin[nPatternLength - nTestIndex - 1];
			for(;;){
				const auto chCandidateFront = itPatternBegin[nPatternLength - nGsrCandidateLength - 1];
				if(chTest == chCandidateFront){
					++nGsrCandidateLength;
					break;
				}
				if(nGsrCandidateLength <= 0){
					break;
				}
				nGsrCandidateLength -= ashGsrOffsetTable[nGsrCandidateLength - 1];
			}
			ashGsrOffsetTable[nTestIndex] = static_cast<short>(nTestIndex - nGsrCandidateLength + 1);
		}
		__attribute__((__aligned__(64))) short ashGsrTable[kGsrTableSize];
		const auto nMaxGsrShift = nGsrTableSize - nGsrCandidateLength;
		ashGsrTable[0] = 0;
		for(std::ptrdiff_t nTestIndex = 1; nTestIndex < nGsrTableSize; ++nTestIndex){
			std::ptrdiff_t nGsrShift = ashGsrOffsetTable[nTestIndex - 1];
			if(nGsrShift != ashGsrOffsetTable[nTestIndex]){
				for(;;){
					const auto nAdjustIndex = nTestIndex - nGsrShift;
					if(nAdjustIndex <= 0){
						break;
					}
					if(ashGsrTable[nAdjustIndex] > nGsrShift){
						ashGsrTable[nAdjustIndex] = static_cast<short>(nGsrShift);
					}
					nGsrShift += ashGsrOffsetTable[nAdjustIndex - 1];
				}
			}
			ashGsrTable[nTestIndex] = static_cast<short>(nMaxGsrShift);
		}

		std::ptrdiff_t nOffset = 0;
		std::ptrdiff_t nKnownMatchEnd = 0;
		std::ptrdiff_t nKnownMatchBegin = 0;
		for(;;){
			if(nTextCount - nOffset < nPatternLength){
				return itTextEnd;
			}
			std::ptrdiff_t nTestIndex = nPatternLength - 1;
			const auto chLast = itTextBegin[nOffset + nTestIndex];
			for(;;){
				const auto chText = itTextBegin[nOffset + nTestIndex];
				const auto chPattern = itPatternBegin[nTestIndex];
				if(chText != chPattern){
					break;
				}
				if(nTestIndex == nKnownMatchEnd){
					nTestIndex = nKnownMatchBegin;
				}
				if(nTestIndex <= 0){
					return itTextBegin + nOffset;
				}
				--nTestIndex;
			}
			const auto nSuffixLength = nPatternLength - nTestIndex - 1;
			const std::ptrdiff_t nBcrShift = ashBcrTable[static_cast<std::make_unsigned_t<decltype(chLast)>>(chLast) % kBcrTableSize];
			const std::ptrdiff_t nGsrShift = (nSuffixLength < nGsrTableSize) ? ashGsrTable[nSuffixLength] : 0;
			if(_MCFCRT_EXPECT(nBcrShift > nGsrShift)){
				nOffset += nBcrShift;
				nKnownMatchEnd = 0;
				nKnownMatchBegin = 0;
			} else {
				nOffset += nGsrShift;
				nKnownMatchEnd = nPatternLength - nGsrShift;
				nKnownMatchBegin = nKnownMatchEnd - nSuffixLength;
			}
		}
	}

	template<typename TextBeginT, typename TextEndT, typename PatternT>
	TextBeginT FindRepeat(TextBeginT itTextBegin, TextEndT itTextEnd, const PatternT &chPattern, std::size_t uPatternLength){
		const auto nPatternLength = static_cast<std::ptrdiff_t>(uPatternLength);
		if(nPatternLength < 0){
			return itTextEnd;
		}
		if(nPatternLength == 0){
			return itTextBegin;
		}
		const auto nTextCount = static_cast<std::ptrdiff_t>(itTextEnd - itTextBegin);
		if(nTextCount < nPatternLength){
			return itTextEnd;
		}

		std::ptrdiff_t nOffset = 0;
		std::ptrdiff_t nKnownMatchEnd = 0;
		for(;;){
			if(nTextCount - nOffset < nPatternLength){
				return itTextEnd;
			}
			std::ptrdiff_t nTestIndex = nPatternLength - 1;
			for(;;){
				const auto chText = itTextBegin[nOffset + nTestIndex];
				if(chText != chPattern){
					break;
				}
				if(nTestIndex <= nKnownMatchEnd){
					return itTextBegin + nOffset;
				}
				--nTestIndex;
			}
			const auto nShift = nTestIndex + 1;
			nOffset += nShift;
			nKnownMatchEnd = nPatternLength - nShift;
		}
	}
}

}

#endif
