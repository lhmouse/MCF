// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_ALGORITHMS_HPP_
#define MCF_ALGORITHMS_HPP_

#include <type_traits>
#include <utility>
#include <iterator>
#include <cstddef>
#include <cstring>

namespace MCF {

namespace Impl {
	template<typename OutputIterator, typename InputIterator, typename Value,
		typename TraitHelper = void
		>
	struct CopyTrait {
		static OutputIterator DoCopy(
			OutputIterator itOutputBegin,
			InputIterator itInputBegin,
			typename std::common_type<InputIterator>::type itInputEnd
		){
			while(itInputBegin != itInputEnd){
				*itOutputBegin = *itInputBegin;
				++itOutputBegin;
				++itInputBegin;
			}
			return std::move(itOutputBegin);
		}
		static std::pair<OutputIterator, InputIterator> DoCopyN(
			OutputIterator itOutputBegin,
			InputIterator itInputBegin,
			std::size_t uCount
		){
			for(auto i = uCount; i; --i){
				*itOutputBegin = *itInputBegin;
				++itOutputBegin;
				++itInputBegin;
			}
			return std::make_pair(std::move(itOutputBegin), std::move(itInputBegin));
		}
		static OutputIterator DoCopyBackward(
			OutputIterator itOutputEnd,
			InputIterator itInputBegin,
			typename std::common_type<InputIterator>::type itInputEnd
		){
			while(itInputBegin != itInputEnd){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = *itInputEnd;
			}
			return std::move(itOutputEnd);
		}
		static std::pair<OutputIterator, InputIterator> DoCopyBackwardN(
			OutputIterator itOutputEnd,
			std::size_t uCount,
			InputIterator itInputEnd
		){
			for(auto i = uCount; i; --i){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = *itInputEnd;
			}
			return std::make_pair(std::move(itOutputEnd), std::move(itInputEnd));
		}

		static OutputIterator DoMove(
			OutputIterator itOutputBegin,
			InputIterator itInputBegin,
			typename std::common_type<InputIterator>::type itInputEnd
		){
			while(itInputBegin != itInputEnd){
				*itOutputBegin = std::move(*itInputBegin);
				++itOutputBegin;
				++itInputBegin;
			}
			return std::move(itOutputBegin);
		}
		static std::pair<OutputIterator, InputIterator> DoMoveN(
			OutputIterator itOutputBegin,
			InputIterator itInputBegin,
			std::size_t uCount
		){
			for(auto i = uCount; i; --i){
				*itOutputBegin = std::move(*itInputBegin);
				++itOutputBegin;
				++itInputBegin;
			}
			return std::make_pair(std::move(itOutputBegin), std::move(itInputBegin));
		}
		static OutputIterator DoMoveBackward(
			OutputIterator itOutputEnd,
			InputIterator itInputBegin,
			typename std::common_type<InputIterator>::type itInputEnd
		){
			while(itInputBegin != itInputEnd){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = std::move(*itInputEnd);
			}
			return std::move(itOutputEnd);
		}
		static std::pair<OutputIterator, InputIterator> DoMoveBackwardN(
			OutputIterator itOutputEnd,
			std::size_t uCount,
			InputIterator itInputEnd
		){
			for(auto i = uCount; i; --i){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = std::move(*itInputEnd);
			}
			return std::make_pair(std::move(itOutputEnd), std::move(itInputEnd));
		}
	};

	template<typename TriviallyCopyable, typename InputIterator>
	struct CopyTrait<TriviallyCopyable *, InputIterator, TriviallyCopyable,
		typename std::enable_if<
//			std::is_trivially_copyable<TriviallyCopyable>::value &&
				std::is_trivial<TriviallyCopyable>::value &&			// TODO: Fuck GCC.
				!std::is_volatile<TriviallyCopyable>::value &&			//
				std::is_move_constructible<TriviallyCopyable>::value &&	//
			std::is_pointer<InputIterator>::value &&
			std::is_same<
				typename std::remove_const<
					typename std::remove_pointer<InputIterator>::type
					>::type,
				TriviallyCopyable
				>::value
			>::type
		>
	{
		typedef TriviallyCopyable *OutputIterator;

		static OutputIterator DoCopy(
			OutputIterator itOutputBegin,
			InputIterator itInputBegin,
			typename std::common_type<InputIterator>::type itInputEnd
		) noexcept {
			const auto uCount = (std::size_t)(itInputEnd - itInputBegin);
			std::memmove(
				itOutputBegin,
				itInputBegin,
				uCount * sizeof(TriviallyCopyable)
			);
			return itOutputBegin + uCount;
		}
		static std::pair<OutputIterator, InputIterator> DoCopyN(
			OutputIterator itOutputBegin,
			InputIterator itInputBegin,
			std::size_t uCount
		) noexcept {
			std::memmove(
				itOutputBegin,
				itInputBegin,
				uCount * sizeof(TriviallyCopyable)
			);
			return std::make_pair(itOutputBegin + uCount, itInputBegin + uCount);
		}
		static OutputIterator DoCopyBackward(
			OutputIterator itOutputEnd,
			InputIterator itInputBegin,
			typename std::common_type<InputIterator>::type itInputEnd
		) noexcept {
			const auto uCount = (std::size_t)(itInputEnd - itInputBegin);
			std::memmove(
				itOutputEnd - uCount,
				itInputEnd - uCount,
				uCount * sizeof(TriviallyCopyable)
			);
			return itOutputEnd - uCount;
		}
		static std::pair<OutputIterator, InputIterator> DoCopyBackwardN(
			OutputIterator itOutputEnd,
			std::size_t uCount,
			InputIterator itInputEnd
		) noexcept {
			std::memmove(
				itOutputEnd - uCount,
				itInputEnd - uCount,
				uCount * sizeof(TriviallyCopyable)
			);
			return std::make_pair(itOutputEnd - uCount, itInputEnd - uCount);
		}

		static OutputIterator DoMove(
			OutputIterator itOutputBegin,
			InputIterator itInputBegin,
			typename std::common_type<InputIterator>::type itInputEnd
		) noexcept {
			return DoCopy(
				std::move(itOutputBegin),
				std::move(itInputBegin),
				std::move(itInputEnd)
			);
		}
		static std::pair<OutputIterator, InputIterator> DoMoveN(
			OutputIterator itOutputBegin,
			InputIterator itInputBegin,
			std::size_t uCount
		) noexcept {
			return DoCopyN(
				std::move(itOutputBegin),
				std::move(itInputBegin),
				uCount
			);
		}
		static OutputIterator DoMoveBackward(
			OutputIterator itOutputEnd,
			InputIterator itInputBegin,
			typename std::common_type<InputIterator>::type itInputEnd
		) noexcept {
			return DoCopyBackward(
				std::move(itOutputEnd),
				std::move(itInputBegin),
				std::move(itInputEnd)
			);
		}
		static std::pair<OutputIterator, InputIterator> DoMoveBackwardN(
			OutputIterator itOutputEnd,
			std::size_t uCount,
			InputIterator itInputEnd
		) noexcept {
			return DoCopyBackwardN(
				std::move(itOutputEnd),
				uCount,
				std::move(itInputEnd)
			);
		}
	};
}

template<typename OutputIterator, typename InputIterator>
inline OutputIterator Copy(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
){
	return Impl::CopyTrait<OutputIterator, InputIterator,
		typename std::remove_reference<
			typename std::iterator_traits<InputIterator>::value_type
			>::type
		>::DoCopy(
			std::move(itOutputBegin),
			std::move(itInputBegin),
			std::move(itInputEnd)
		);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> CopyN(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	std::size_t uCount
){
	return Impl::CopyTrait<OutputIterator, InputIterator,
		typename std::remove_reference<
			typename std::iterator_traits<InputIterator>::value_type
			>::type
		>::DoCopyN(
			std::move(itOutputBegin),
			std::move(itInputBegin),
			uCount
		);
}
template<typename OutputIterator, typename InputIterator>
inline OutputIterator CopyBackward(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
){
	return Impl::CopyTrait<OutputIterator, InputIterator,
		typename std::remove_reference<
			typename std::iterator_traits<InputIterator>::value_type
			>::type
		>::DoCopyBackward(
			std::move(itOutputEnd),
			std::move(itInputBegin),
			std::move(itInputEnd)
		);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> CopyBackwardN(
	OutputIterator itOutputEnd,
	std::size_t uCount,
	InputIterator itInputEnd
){
	return Impl::CopyTrait<OutputIterator, InputIterator,
		typename std::remove_reference<
			typename std::iterator_traits<InputIterator>::value_type
			>::type
		>::DoCopyBackwardN(
			std::move(itOutputEnd),
			uCount,
			std::move(itInputEnd)
		);
}

template<typename OutputIterator, typename InputIterator>
inline OutputIterator ReverseCopy(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
){
	return Copy(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator>(itInputEnd),
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> ReverseCopyN(
	OutputIterator itOutputBegin,
	std::size_t uCount,
	InputIterator itInputEnd
){
	auto vResult = CopyN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator>(itInputEnd),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIterator, typename InputIterator>
inline OutputIterator ReverseCopyBackward(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
){
	return CopyBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputIterator>(itInputEnd),
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> ReverseCopyBackwardN(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	std::size_t uCount
){
	auto vResult = CopyBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

template<typename OutputIterator, typename InputIterator>
inline OutputIterator Move(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
){
	return Impl::CopyTrait<OutputIterator, InputIterator,
		typename std::remove_reference<
			typename std::iterator_traits<InputIterator>::value_type
			>::type
		>::DoMove(
			std::move(itOutputBegin),
			std::move(itInputBegin),
			std::move(itInputEnd)
		);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> MoveN(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	std::size_t uCount
){
	return Impl::CopyTrait<OutputIterator, InputIterator,
		typename std::remove_reference<
			typename std::iterator_traits<InputIterator>::value_type
			>::type
		>::DoMoveN(
			std::move(itOutputBegin),
			std::move(itInputBegin),
			uCount
		);
}
template<typename OutputIterator, typename InputIterator>
inline OutputIterator MoveBackward(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
){
	return Impl::CopyTrait<OutputIterator, InputIterator,
		typename std::remove_reference<
			typename std::iterator_traits<InputIterator>::value_type
			>::type
		>::DoMoveBackward(
			std::move(itOutputEnd),
			std::move(itInputBegin),
			std::move(itInputEnd)
		);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> MoveBackwardN(
	OutputIterator itOutputEnd,
	std::size_t uCount,
	InputIterator itInputEnd
){
	return Impl::CopyTrait<OutputIterator, InputIterator,
		typename std::remove_reference<
			typename std::iterator_traits<InputIterator>::value_type
			>::type
		>::DoMoveBackwardN(
			std::move(itOutputEnd),
			uCount,
			std::move(itInputEnd)
		);
}

template<typename OutputIterator, typename InputIterator>
inline OutputIterator ReverseMove(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
){
	return Move(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator>(itInputEnd),
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> ReverseMoveN(
	OutputIterator itOutputBegin,
	std::size_t uCount,
	InputIterator itInputEnd
){
	auto vResult = MoveN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator>(itInputEnd),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIterator, typename InputIterator>
inline OutputIterator ReverseMoveBackward(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
){
	return MoveBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputIterator>(itInputEnd),
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> ReverseMoveBackwardN(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	std::size_t uCount
){
	auto vResult = MoveBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

template<typename Value = void, typename OutputIterator, typename... Params>
inline OutputIterator Fill(
	OutputIterator itOutputBegin,
	typename std::common_type<OutputIterator>::type itOutputEnd,
	const Params &... vParams
){
	typedef typename std::conditional<
		std::is_void<Value>::value,
		typename std::remove_reference<decltype(*itOutputBegin)>::type, Value
		>::type ToConstruct;

	while(itOutputBegin != itOutputEnd){
		*itOutputBegin = ToConstruct(vParams...);
		++itOutputBegin;
	}
	return std::move(itOutputBegin);
}
template<typename Value = void, typename OutputIterator, typename... Params>
inline OutputIterator FillN(
	OutputIterator itOutputBegin,
	std::size_t uCount,
	const Params &... vParams
){
	typedef typename std::conditional<
		std::is_void<Value>::value,
		typename std::remove_reference<decltype(*itOutputBegin)>::type, Value
		>::type ToConstruct;

	for(auto i = uCount; i; --i){
		*itOutputBegin = ToConstruct(vParams...);
		++itOutputBegin;
	}
	return std::move(itOutputBegin);
}

}

#endif
