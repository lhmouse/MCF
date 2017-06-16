// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_TRAITS_HPP_
#define MCF_CORE_STRING_TRAITS_HPP_

#include <type_traits>
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
		using Self = std::decay_t<decltype(*itSelfBegin)>;

		auto itSelf = itSelfBegin;

		for(;;){
			const Self chSelf = *itSelf;
			if(chSelf == Self()){
				return itSelf;
			}
			++itSelf;
		}
	}

	template<typename SelfBeginT, typename SelfEndT, typename ComparandBeginT, typename ComparandEndT>
	int Compare(SelfBeginT itSelfBegin, SelfEndT itSelfEnd, ComparandBeginT itComparandBegin, ComparandEndT itComparandEnd){
		using Self = std::decay_t<decltype(*itSelfBegin)>;
		using Comparand = std::decay_t<decltype(*itComparandBegin)>;

		auto itSelf = itSelfBegin;
		auto itComparand = itComparandBegin;

		for(;;){
			const int nEndMarkSelf = static_cast<bool>(itSelf == itSelfEnd);
			const int nEndMarkComparand = -static_cast<bool>(itComparand == itComparandEnd);
			if(nEndMarkSelf || nEndMarkComparand){
				return nEndMarkSelf + nEndMarkComparand;
			}
			const Self chSelf = *itSelf;
			const Comparand chComparand = *itComparand;
			if(chSelf != chComparand){
				return (static_cast<std::make_unsigned_t<Self>>(chSelf) < static_cast<std::make_unsigned_t<Comparand>>(chComparand)) ? -1 : 1;
			}
			++itSelf;
			++itComparand;
		}
	}

	template<typename SelfBeginT, typename SelfEndT, typename ComparandT>
	SelfBeginT FindRepeat(SelfBeginT itSelfBegin, SelfEndT itSelfEnd, const ComparandT &chComparand, std::size_t uComparandCount){
		using Self = std::decay_t<decltype(*itSelfBegin)>;

		if(uComparandCount == 0){
			return itSelfBegin;
		}
		const auto uSelfCount = static_cast<std::size_t>(std::distance(itSelfBegin, itSelfEnd));
		if(uSelfCount < uComparandCount){
			return itSelfEnd;
		}

		auto itSelf = itSelfBegin;
		std::ptrdiff_t nMatchCount = 0;
		decltype(itSelfEnd) itMatch;

		for(;;){
			if(itSelf == itSelfEnd){
				return itSelfEnd;
			}
			const Self chSelf = *itSelf;
			if(chSelf == chComparand){
				if(nMatchCount == 0){
					itMatch = itSelf;
				}
				if(static_cast<std::size_t>(++nMatchCount) >= uComparandCount){
					return itMatch;
				}
			} else {
				nMatchCount = 0;
			}
			++itSelf;
		}
	}

	template<typename SelfBeginT, typename SelfEndT, typename ComparandBeginT, typename ComparandEndT>
	SelfBeginT FindSpan(SelfBeginT itSelfBegin, SelfEndT itSelfEnd, ComparandBeginT itComparandBegin, ComparandEndT itComparandEnd){
	//	using Self = std::decay_t<decltype(*itSelfBegin)>;
	//	using Comparand = std::decay_t<decltype(*itComparandBegin)>;

		const auto uComparandCount = static_cast<std::size_t>(std::distance(itComparandBegin, itComparandEnd));
		if(uComparandCount == 0){
			return itSelfBegin;
		}
		const auto uSelfCount = static_cast<std::size_t>(std::distance(itSelfBegin, itSelfEnd));
		if(uSelfCount < uComparandCount){
			return itSelfEnd;
		}

		// TODO
		return std::search(itSelfBegin, itSelfEnd, itComparandBegin, itComparandEnd);
	}
}

}

#endif
