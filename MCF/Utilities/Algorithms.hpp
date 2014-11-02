// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_ALGORITHMS_HPP_
#define MCF_UTILITIES_ALGORITHMS_HPP_

#include <type_traits>
#include <utility>
#include <iterator>
#include <cstddef>
#include <cstring>

namespace MCF {

namespace Impl {
	template<typename OutputIteratorT, typename InputIteratorT, typename ValueT,
		typename TraitHelper = void>
	struct CopyTrait {
		static OutputIteratorT DoCopy(
			OutputIteratorT itOutputBegin,
			InputIteratorT itInputBegin,
			std::common_type_t<InputIteratorT> itInputEnd
		){
			while(itInputBegin != itInputEnd){
				*itOutputBegin = *itInputBegin;
				++itOutputBegin;
				++itInputBegin;
			}
			return std::move(itOutputBegin);
		}
		static std::pair<OutputIteratorT, InputIteratorT> DoCopyN(
			OutputIteratorT itOutputBegin,
			InputIteratorT itInputBegin,
			std::size_t uCount
		){
			for(auto i = uCount; i; --i){
				*itOutputBegin = *itInputBegin;
				++itOutputBegin;
				++itInputBegin;
			}
			return std::make_pair(std::move(itOutputBegin), std::move(itInputBegin));
		}
		static OutputIteratorT DoCopyBackward(
			OutputIteratorT itOutputEnd,
			InputIteratorT itInputBegin,
			std::common_type_t<InputIteratorT> itInputEnd
		){
			while(itInputBegin != itInputEnd){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = *itInputEnd;
			}
			return std::move(itOutputEnd);
		}
		static std::pair<OutputIteratorT, InputIteratorT> DoCopyBackwardN(
			OutputIteratorT itOutputEnd,
			std::size_t uCount,
			InputIteratorT itInputEnd
		){
			for(auto i = uCount; i; --i){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = *itInputEnd;
			}
			return std::make_pair(std::move(itOutputEnd), std::move(itInputEnd));
		}

		static OutputIteratorT DoMove(
			OutputIteratorT itOutputBegin,
			InputIteratorT itInputBegin,
			std::common_type_t<InputIteratorT> itInputEnd
		){
			while(itInputBegin != itInputEnd){
				*itOutputBegin = std::move(*itInputBegin);
				++itOutputBegin;
				++itInputBegin;
			}
			return std::move(itOutputBegin);
		}
		static std::pair<OutputIteratorT, InputIteratorT> DoMoveN(
			OutputIteratorT itOutputBegin,
			InputIteratorT itInputBegin,
			std::size_t uCount
		){
			for(auto i = uCount; i; --i){
				*itOutputBegin = std::move(*itInputBegin);
				++itOutputBegin;
				++itInputBegin;
			}
			return std::make_pair(std::move(itOutputBegin), std::move(itInputBegin));
		}
		static OutputIteratorT DoMoveBackward(
			OutputIteratorT itOutputEnd,
			InputIteratorT itInputBegin,
			std::common_type_t<InputIteratorT> itInputEnd
		){
			while(itInputBegin != itInputEnd){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = std::move(*itInputEnd);
			}
			return std::move(itOutputEnd);
		}
		static std::pair<OutputIteratorT, InputIteratorT> DoMoveBackwardN(
			OutputIteratorT itOutputEnd,
			std::size_t uCount,
			InputIteratorT itInputEnd
		){
			for(auto i = uCount; i; --i){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = std::move(*itInputEnd);
			}
			return std::make_pair(std::move(itOutputEnd), std::move(itInputEnd));
		}
	};

	template<typename TriviallyCopyableT, typename InputIteratorT>
	struct CopyTrait<TriviallyCopyableT *, InputIteratorT, TriviallyCopyableT,
		std::enable_if_t<
//			std::is_trivially_copyable<TriviallyCopyableT>::value &&
				std::is_trivial<TriviallyCopyableT>::value &&				// TODO: Fuck GCC.
				!std::is_volatile<TriviallyCopyableT>::value &&				//
				std::is_copy_constructible<TriviallyCopyableT>::value &&	//
				std::is_move_constructible<TriviallyCopyableT>::value &&	//
			std::is_pointer<InputIteratorT>::value &&
			std::is_same<
				std::remove_const_t<std::remove_pointer_t<InputIteratorT>>,
				TriviallyCopyableT
				>::value
			>
		>
	{
		typedef TriviallyCopyableT *OutputIteratorT;

		static OutputIteratorT DoCopy(
			OutputIteratorT itOutputBegin,
			InputIteratorT itInputBegin,
			std::common_type_t<InputIteratorT> itInputEnd
		) noexcept {
			const auto uCount = (std::size_t)(itInputEnd - itInputBegin);
			std::memmove(
				itOutputBegin,
				itInputBegin,
				uCount * sizeof(TriviallyCopyableT)
			);
			return itOutputBegin + uCount;
		}
		static std::pair<OutputIteratorT, InputIteratorT> DoCopyN(
			OutputIteratorT itOutputBegin,
			InputIteratorT itInputBegin,
			std::size_t uCount
		) noexcept {
			std::memmove(
				itOutputBegin,
				itInputBegin,
				uCount * sizeof(TriviallyCopyableT)
			);
			return std::make_pair(itOutputBegin + uCount, itInputBegin + uCount);
		}
		static OutputIteratorT DoCopyBackward(
			OutputIteratorT itOutputEnd,
			InputIteratorT itInputBegin,
			std::common_type_t<InputIteratorT> itInputEnd
		) noexcept {
			const auto uCount = (std::size_t)(itInputEnd - itInputBegin);
			std::memmove(
				itOutputEnd - uCount,
				itInputEnd - uCount,
				uCount * sizeof(TriviallyCopyableT)
			);
			return itOutputEnd - uCount;
		}
		static std::pair<OutputIteratorT, InputIteratorT> DoCopyBackwardN(
			OutputIteratorT itOutputEnd,
			std::size_t uCount,
			InputIteratorT itInputEnd
		) noexcept {
			std::memmove(
				itOutputEnd - uCount,
				itInputEnd - uCount,
				uCount * sizeof(TriviallyCopyableT)
			);
			return std::make_pair(itOutputEnd - uCount, itInputEnd - uCount);
		}

		static OutputIteratorT DoMove(
			OutputIteratorT itOutputBegin,
			InputIteratorT itInputBegin,
			std::common_type_t<InputIteratorT> itInputEnd
		) noexcept {
			return DoCopy(
				std::move(itOutputBegin),
				std::move(itInputBegin),
				std::move(itInputEnd)
			);
		}
		static std::pair<OutputIteratorT, InputIteratorT> DoMoveN(
			OutputIteratorT itOutputBegin,
			InputIteratorT itInputBegin,
			std::size_t uCount
		) noexcept {
			return DoCopyN(
				std::move(itOutputBegin),
				std::move(itInputBegin),
				uCount
			);
		}
		static OutputIteratorT DoMoveBackward(
			OutputIteratorT itOutputEnd,
			InputIteratorT itInputBegin,
			std::common_type_t<InputIteratorT> itInputEnd
		) noexcept {
			return DoCopyBackward(
				std::move(itOutputEnd),
				std::move(itInputBegin),
				std::move(itInputEnd)
			);
		}
		static std::pair<OutputIteratorT, InputIteratorT> DoMoveBackwardN(
			OutputIteratorT itOutputEnd,
			std::size_t uCount,
			InputIteratorT itInputEnd
		) noexcept {
			return DoCopyBackwardN(
				std::move(itOutputEnd),
				uCount,
				std::move(itInputEnd)
			);
		}
	};
}

template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT Copy(
	OutputIteratorT itOutputBegin,
	InputIteratorT itInputBegin,
	std::common_type_t<InputIteratorT> itInputEnd
){
	return Impl::CopyTrait<OutputIteratorT, InputIteratorT,
		std::remove_reference_t<
			typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoCopy
	(
		std::move(itOutputBegin),
		std::move(itInputBegin),
		std::move(itInputEnd)
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT> CopyN(
	OutputIteratorT itOutputBegin,
	InputIteratorT itInputBegin,
	std::size_t uCount
){
	return Impl::CopyTrait<OutputIteratorT, InputIteratorT,
		std::remove_reference_t<
			typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoCopyN
	(
		std::move(itOutputBegin),
		std::move(itInputBegin),
		uCount
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT CopyBackward(
	OutputIteratorT itOutputEnd,
	InputIteratorT itInputBegin,
	std::common_type_t<InputIteratorT> itInputEnd
){
	return Impl::CopyTrait<OutputIteratorT, InputIteratorT,
		std::remove_reference_t<
			typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoCopyBackward
	(
		std::move(itOutputEnd),
		std::move(itInputBegin),
		std::move(itInputEnd)
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT> CopyBackwardN(
	OutputIteratorT itOutputEnd,
	std::size_t uCount,
	InputIteratorT itInputEnd
){
	return Impl::CopyTrait<OutputIteratorT, InputIteratorT,
		std::remove_reference_t<
			typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoCopyBackwardN
	(
		std::move(itOutputEnd),
		uCount,
		std::move(itInputEnd)
	);
}

template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT ReverseCopy(
	OutputIteratorT itOutputBegin,
	InputIteratorT itInputBegin,
	std::common_type_t<InputIteratorT> itInputEnd
){
	return Copy(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIteratorT>(itInputEnd),
		std::reverse_iterator<InputIteratorT>(itInputBegin)
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT> ReverseCopyN(
	OutputIteratorT itOutputBegin,
	std::size_t uCount,
	InputIteratorT itInputEnd
){
	auto vResult = CopyN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIteratorT>(itInputEnd),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT ReverseCopyBackward(
	OutputIteratorT itOutputEnd,
	InputIteratorT itInputBegin,
	std::common_type_t<InputIteratorT> itInputEnd
){
	return CopyBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputIteratorT>(itInputEnd),
		std::reverse_iterator<InputIteratorT>(itInputBegin)
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT> ReverseCopyBackwardN(
	OutputIteratorT itOutputEnd,
	InputIteratorT itInputBegin,
	std::size_t uCount
){
	auto vResult = CopyBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputIteratorT>(itInputBegin)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT Move(
	OutputIteratorT itOutputBegin,
	InputIteratorT itInputBegin,
	std::common_type_t<InputIteratorT> itInputEnd
){
	return Impl::CopyTrait<OutputIteratorT, InputIteratorT,
		std::remove_reference_t<
			typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoMove
	(
		std::move(itOutputBegin),
		std::move(itInputBegin),
		std::move(itInputEnd)
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT> MoveN(
	OutputIteratorT itOutputBegin,
	InputIteratorT itInputBegin,
	std::size_t uCount
){
	return Impl::CopyTrait<OutputIteratorT, InputIteratorT,
		std::remove_reference_t<
			typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoMoveN
	(
		std::move(itOutputBegin),
		std::move(itInputBegin),
		uCount
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT MoveBackward(
	OutputIteratorT itOutputEnd,
	InputIteratorT itInputBegin,
	std::common_type_t<InputIteratorT> itInputEnd
){
	return Impl::CopyTrait<OutputIteratorT, InputIteratorT,
		std::remove_reference_t<
			typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoMoveBackward
	(
		std::move(itOutputEnd),
		std::move(itInputBegin),
		std::move(itInputEnd)
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT> MoveBackwardN(
	OutputIteratorT itOutputEnd,
	std::size_t uCount,
	InputIteratorT itInputEnd
){
	return Impl::CopyTrait<OutputIteratorT, InputIteratorT,
		std::remove_reference_t<
			typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoMoveBackwardN
	(
		std::move(itOutputEnd),
		uCount,
		std::move(itInputEnd)
	);
}

template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT ReverseMove(
	OutputIteratorT itOutputBegin,
	InputIteratorT itInputBegin,
	std::common_type_t<InputIteratorT> itInputEnd
){
	return Move(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIteratorT>(itInputEnd),
		std::reverse_iterator<InputIteratorT>(itInputBegin)
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT> ReverseMoveN(
	OutputIteratorT itOutputBegin,
	std::size_t uCount,
	InputIteratorT itInputEnd
){
	auto vResult = MoveN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIteratorT>(itInputEnd),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT ReverseMoveBackward(
	OutputIteratorT itOutputEnd,
	InputIteratorT itInputBegin,
	std::common_type_t<InputIteratorT> itInputEnd
){
	return MoveBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputIteratorT>(itInputEnd),
		std::reverse_iterator<InputIteratorT>(itInputBegin)
	);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT> ReverseMoveBackwardN(
	OutputIteratorT itOutputEnd,
	InputIteratorT itInputBegin,
	std::size_t uCount
){
	auto vResult = MoveBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputIteratorT>(itInputBegin)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
inline OutputIteratorT Fill(
	OutputIteratorT itOutputBegin,
	std::common_type_t<OutputIteratorT> itOutputEnd,
	const ParamsT &...vParams
){
	using ToConstruct = std::conditional_t<std::is_void<ValueT>::value,
		std::remove_reference_t<decltype(*itOutputBegin)>, ValueT>;

	while(itOutputBegin != itOutputEnd){
		*itOutputBegin = ToConstruct(vParams...);
		++itOutputBegin;
	}
	return std::move(itOutputBegin);
}
template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
inline OutputIteratorT FillN(
	OutputIteratorT itOutputBegin,
	std::size_t uCount,
	const ParamsT &...vParams
){
	using ToConstruct = std::conditional_t<std::is_void<ValueT>::value,
		std::remove_reference_t<decltype(*itOutputBegin)>, ValueT>;

	for(auto i = uCount; i; --i){
		*itOutputBegin = ToConstruct(vParams...);
		++itOutputBegin;
	}
	return std::move(itOutputBegin);
}

}

#endif
