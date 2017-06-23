// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_TRAITS_HPP_
#define MCF_CORE_STRING_TRAITS_HPP_

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

	template<typename TextBeginT>
	TextBeginT Define(TextBeginT itTextBegin){
		std::ptrdiff_t nOffset = 0;
		for(;;){
			const auto chText = itTextBegin[nOffset];
			if(chText == decltype(chText)()){
				return itTextBegin + nOffset;
			}
			++nOffset;
		}
	}

	template<typename TextBeginT, typename TextEndT, typename PatternBeginT, typename PatternEndT>
	int Compare(TextBeginT itTextBegin, TextEndT itTextEnd, PatternBeginT itPatternBegin, PatternEndT itPatternEnd){
		std::ptrdiff_t nOffset = 0;
		for(;;){
			const bool bEndMarkText = itTextBegin + nOffset == itTextEnd;
			const bool bEndMarkPattern = itPatternBegin + nOffset == itPatternEnd;
			if(bEndMarkText || bEndMarkPattern){
				return bEndMarkText - bEndMarkPattern;
			}
			const auto chText = itTextBegin[nOffset];
			const auto chPattern = itPatternBegin[nOffset];
			if(chText != chPattern){
				return (static_cast<std::make_unsigned_t<decltype(chText)>>(chText) < static_cast<std::make_unsigned_t<decltype(chPattern)>>(chPattern)) ? -1 : 1;
			}
			++nOffset;
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
		for(;;){
			if(nTextCount - nOffset < nPatternLength){
				return itTextEnd;
			}
			std::ptrdiff_t nTestIndex = nPatternLength - 1;
			for(;;){
				const auto chText = itTextBegin[nOffset + nTestIndex];
				if(chText != chPattern){
					nOffset += nTestIndex + 1;
					break;
				}
				if(nTestIndex == 0){
					return itTextBegin + nOffset;
				}
				--nTestIndex;
			}
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
		__attribute__((__aligned__(64))) short ashGsrTable[kGsrTableSize];
		const std::ptrdiff_t nMaxGsrShift = (nPatternLength <= kGsrTableSize) ? nPatternLength : kGsrTableSize;
		std::ptrdiff_t nGsrCandidateLength = 0;
		ashGsrTable[0] = 1;
		for(std::ptrdiff_t nTestIndex = 1; nTestIndex < nMaxGsrShift; ++nTestIndex){
			const auto chTest = itPatternBegin[nPatternLength - nTestIndex - 1];
			for(;;){
				const auto chCandidateFront = itPatternBegin[nPatternLength - nGsrCandidateLength - 1];
				if(chTest == chCandidateFront){
					++nGsrCandidateLength;
					break;
				}
				if(nGsrCandidateLength == 0){
					break;
				}
				nGsrCandidateLength -= ashGsrTable[nGsrCandidateLength - 1];
			}
			ashGsrTable[nTestIndex] = static_cast<short>(nTestIndex - nGsrCandidateLength + 1);
		}
		std::ptrdiff_t nGsrLastOffset = 1;
		ashGsrTable[0] = 0;
		for(std::ptrdiff_t nTestIndex = 1; nTestIndex < nMaxGsrShift; ++nTestIndex){
			const std::ptrdiff_t nGsrOffset = ashGsrTable[nTestIndex];
			ashGsrTable[nTestIndex] = static_cast<short>(nMaxGsrShift - nGsrCandidateLength);
			if(nGsrLastOffset != nGsrOffset){
				for(std::ptrdiff_t nWriteIndex = nTestIndex - nGsrLastOffset; nWriteIndex > 0; nWriteIndex -= nGsrLastOffset){
					const std::ptrdiff_t nGsrShift = nTestIndex - nWriteIndex;
					if(ashGsrTable[nWriteIndex] <= nGsrShift){
						break;
					}
					ashGsrTable[nWriteIndex] = static_cast<short>(nGsrShift);
				}
			}
			nGsrLastOffset = nGsrOffset;
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
					const auto nSuffixLength = nPatternLength - nTestIndex - 1;
					const std::ptrdiff_t nBcrShift = ashBcrTable[static_cast<std::make_unsigned_t<decltype(chLast)>>(chLast) % kBcrTableSize];
					const std::ptrdiff_t nGsrShift = (nSuffixLength < nMaxGsrShift) ? ashGsrTable[nSuffixLength] : 1;
					if(nBcrShift > nGsrShift){
						nOffset += nBcrShift;
						nKnownMatchEnd = nPatternLength - nBcrShift;
						nKnownMatchBegin = nKnownMatchEnd - 1;
					} else {
						nOffset += nGsrShift;
						nKnownMatchEnd = nPatternLength - nGsrShift;
						nKnownMatchBegin = nKnownMatchEnd - nSuffixLength;
					}
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
		}
	}
}

}

#endif
