// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_TRAITS_HPP_
#define MCF_CORE_STRING_TRAITS_HPP_

#include <type_traits>
#include <iterator>
#include <cstddef>

// TODO: remove std::search
#include <algorithm>

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
		for(std::ptrdiff_t nOffset = 0; ; ++nOffset){
			const auto chSelf = itSelfBegin[nOffset];
			if(chSelf == decltype(chSelf)()){
				return itSelfBegin + nOffset;
			}
		}
	}

	template<typename SelfBeginT, typename SelfEndT, typename ComparandBeginT, typename ComparandEndT>
	int Compare(SelfBeginT itSelfBegin, SelfEndT itSelfEnd, ComparandBeginT itComparandBegin, ComparandEndT itComparandEnd){
		for(std::ptrdiff_t nOffset = 0; ; ++nOffset){
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
		const auto nSelfCount = std::distance(itSelfBegin, itSelfEnd);
		if(nSelfCount < nComparandCount){
			return itSelfEnd;
		}

		for(std::ptrdiff_t nOffset = 0; ; ++nOffset){
			if(nSelfCount - nOffset < nComparandCount){
				return itSelfEnd;
			}
			for(std::ptrdiff_t nLastMatch = nComparandCount - 1; ; --nLastMatch){
				const auto chSelf = itSelfBegin[nOffset + nLastMatch];
				if(chSelf != chComparand){
					nOffset += nLastMatch;
					break;
				}
				if(nLastMatch == 0){
					return itSelfBegin + nOffset;
				}
			}
		}
	}

	template<typename SelfBeginT, typename SelfEndT, typename ComparandBeginT, typename ComparandEndT>
	SelfBeginT FindSpan(SelfBeginT itSelfBegin, SelfEndT itSelfEnd, ComparandBeginT itComparandBegin, ComparandEndT itComparandEnd){
		const auto nComparandCount = std::distance(itComparandBegin, itComparandEnd);
		if(nComparandCount < 0){
			return itSelfEnd;
		}
		if(nComparandCount == 0){
			return itSelfBegin;
		}
		const auto nSelfCount = std::distance(itSelfBegin, itSelfEnd);
		if(nSelfCount < nComparandCount){
			return itSelfEnd;
		}

		// TODO
		return std::search(itSelfBegin, itSelfEnd, itComparandBegin, itComparandEnd);
	}
}

}

#endif
