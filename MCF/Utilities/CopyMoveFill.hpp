// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_COPY_MOVE_FILL_HPP_
#define MCF_UTILITIES_COPY_MOVE_FILL_HPP_

#include <type_traits>
#include <utility>
#include <iterator>
#include <cstddef>
#include <cstring>

namespace MCF {

namespace Impl_CopyMoveFill {
	template<typename OutputIteratorT, typename InputIteratorT, typename ValueT,
		typename TraitHelper = void>
	struct CopyTraits {
		static OutputIteratorT
			DoCopy(OutputIteratorT itOutputBegin,
				InputIteratorT itInputBegin, InputIteratorT itInputEnd)
		{
			while(itInputBegin != itInputEnd){
				*itOutputBegin = *itInputBegin;
				++itOutputBegin;
				++itInputBegin;
			}
			return itOutputBegin;
		}
		static std::pair<OutputIteratorT, InputIteratorT>
			DoCopyN(OutputIteratorT itOutputBegin,
				InputIteratorT itInputBegin, std::size_t uCount)
		{
			for(auto i = uCount; i; --i){
				*itOutputBegin = *itInputBegin;
				++itOutputBegin;
				++itInputBegin;
			}
			return std::make_pair(itOutputBegin, itInputBegin);
		}
		static OutputIteratorT
			DoCopyBackward(OutputIteratorT itOutputEnd,
				InputIteratorT itInputBegin, InputIteratorT itInputEnd)
		{
			while(itInputBegin != itInputEnd){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = *itInputEnd;
			}
			return itOutputEnd;
		}
		static std::pair<OutputIteratorT, InputIteratorT>
			DoCopyNBackward(OutputIteratorT itOutputEnd,
				std::size_t uCount, InputIteratorT itInputEnd)
		{
			for(auto i = uCount; i; --i){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = *itInputEnd;
			}
			return std::make_pair(itOutputEnd, itInputEnd);
		}

		static OutputIteratorT
			DoMove(OutputIteratorT itOutputBegin,
				InputIteratorT itInputBegin, InputIteratorT itInputEnd)
		{
			while(itInputBegin != itInputEnd){
				*itOutputBegin = std::move(*itInputBegin);
				++itOutputBegin;
				++itInputBegin;
			}
			return itOutputBegin;
		}
		static std::pair<OutputIteratorT, InputIteratorT>
			DoMoveN(OutputIteratorT itOutputBegin,
				InputIteratorT itInputBegin, std::size_t uCount)
		{
			for(auto i = uCount; i; --i){
				*itOutputBegin = std::move(*itInputBegin);
				++itOutputBegin;
				++itInputBegin;
			}
			return std::make_pair(itOutputBegin, itInputBegin);
		}
		static OutputIteratorT
			DoMoveBackward(OutputIteratorT itOutputEnd,
				InputIteratorT itInputBegin, InputIteratorT itInputEnd)
		{
			while(itInputBegin != itInputEnd){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = std::move(*itInputEnd);
			}
			return itOutputEnd;
		}
		static std::pair<OutputIteratorT, InputIteratorT>
			DoMoveNBackward(OutputIteratorT itOutputEnd,
				std::size_t uCount, InputIteratorT itInputEnd)
		{
			for(auto i = uCount; i; --i){
				--itOutputEnd;
				--itInputEnd;
				*itOutputEnd = std::move(*itInputEnd);
			}
			return std::make_pair(itOutputEnd, itInputEnd);
		}
	};

	template<typename TriviallyCopyableT, typename InputIteratorT>
	struct CopyTraits<TriviallyCopyableT *, InputIteratorT, TriviallyCopyableT,
		std::enable_if_t<
//			std::is_trivially_copyable<TriviallyCopyableT>::value &&
				std::is_trivial<TriviallyCopyableT>::value &&				// FIXME: Fuck GCC.
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
		using OutputIterator = TriviallyCopyableT *;

		static OutputIterator
			DoCopy(OutputIterator itOutputBegin,
				InputIteratorT itInputBegin, InputIteratorT itInputEnd)
		{
			const auto uCount = (std::size_t)(itInputEnd - itInputBegin);
			std::memmove(itOutputBegin, itInputBegin, uCount * sizeof(TriviallyCopyableT));
			return itOutputBegin + uCount;
		}
		static std::pair<OutputIterator, InputIteratorT>
			DoCopyN(OutputIterator itOutputBegin,
				InputIteratorT itInputBegin, std::size_t uCount)
		{
			std::memmove(itOutputBegin, itInputBegin, uCount * sizeof(TriviallyCopyableT));
			return std::make_pair(itOutputBegin + uCount, itInputBegin + uCount);
		}
		static OutputIterator
			DoCopyBackward(OutputIterator itOutputEnd,
				InputIteratorT itInputBegin, InputIteratorT itInputEnd)
		{
			const auto uCount = (std::size_t)(itInputEnd - itInputBegin);
			std::memmove(itOutputEnd - uCount, itInputEnd - uCount, uCount * sizeof(TriviallyCopyableT));
			return itOutputEnd - uCount;
		}
		static std::pair<OutputIterator, InputIteratorT>
			DoCopyNBackward(OutputIterator itOutputEnd,
				std::size_t uCount, InputIteratorT itInputEnd)
		{
			std::memmove(itOutputEnd - uCount, itInputEnd - uCount, uCount * sizeof(TriviallyCopyableT));
			return std::make_pair(itOutputEnd - uCount, itInputEnd - uCount);
		}

		static OutputIterator
			DoMove(OutputIterator itOutputBegin,
				InputIteratorT itInputBegin, InputIteratorT itInputEnd)
		{
			return DoCopy(itOutputBegin, itInputBegin, itInputEnd);
		}
		static std::pair<OutputIterator, InputIteratorT>
			DoMoveN(OutputIterator itOutputBegin,
				InputIteratorT itInputBegin, std::size_t uCount)
		{
			return DoCopyN(itOutputBegin, itInputBegin, uCount);
		}
		static OutputIterator
			DoMoveBackward(OutputIterator itOutputEnd,
				InputIteratorT itInputBegin, InputIteratorT itInputEnd)
		{
			return DoCopyBackward(itOutputEnd, itInputBegin, itInputEnd);
		}
		static std::pair<OutputIterator, InputIteratorT>
			DoMoveNBackward(OutputIterator itOutputEnd,
				std::size_t uCount, InputIteratorT itInputEnd)
		{
			return DoCopyNBackward(itOutputEnd, uCount, itInputEnd);
		}
	};
}

template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT
	Copy(OutputIteratorT itOutputBegin,
		InputIteratorT itInputBegin, std::common_type_t<InputIteratorT> itInputEnd)
{
	return Impl_CopyMoveFill::CopyTraits<OutputIteratorT, InputIteratorT,
		std::decay_t<typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoCopy(itOutputBegin, itInputBegin, itInputEnd);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT>
	CopyN(OutputIteratorT itOutputBegin,
		InputIteratorT itInputBegin, std::size_t uCount)
{
	return Impl_CopyMoveFill::CopyTraits<OutputIteratorT, InputIteratorT,
		std::decay_t<typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoCopyN(itOutputBegin, itInputBegin, uCount);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT
	CopyBackward(OutputIteratorT itOutputEnd,
		std::common_type_t<InputIteratorT> itInputBegin, InputIteratorT itInputEnd)
{
	return Impl_CopyMoveFill::CopyTraits<OutputIteratorT, InputIteratorT,
		std::decay_t<typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoCopyBackward(itOutputEnd, itInputBegin, itInputEnd);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT>
	CopyNBackward(OutputIteratorT itOutputEnd,
		std::size_t uCount, InputIteratorT itInputEnd)
{
	return Impl_CopyMoveFill::CopyTraits<OutputIteratorT, InputIteratorT,
		std::decay_t<typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoCopyNBackward(itOutputEnd, uCount, itInputEnd);
}

template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT
	Move(OutputIteratorT itOutputBegin,
		InputIteratorT itInputBegin, std::common_type_t<InputIteratorT> itInputEnd)
{
	return Impl_CopyMoveFill::CopyTraits<OutputIteratorT, InputIteratorT,
		std::decay_t<typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoMove(itOutputBegin, itInputBegin, itInputEnd);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT>
	MoveN(OutputIteratorT itOutputBegin,
		InputIteratorT itInputBegin, std::size_t uCount)
{
	return Impl_CopyMoveFill::CopyTraits<OutputIteratorT, InputIteratorT,
		std::decay_t<typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoMoveN(itOutputBegin, itInputBegin, uCount);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT
	MoveBackward(OutputIteratorT itOutputEnd,
		std::common_type_t<InputIteratorT> itInputBegin, InputIteratorT itInputEnd)
{
	return Impl_CopyMoveFill::CopyTraits<OutputIteratorT, InputIteratorT,
		std::decay_t<typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoMoveBackward(itOutputEnd, itInputBegin, itInputEnd);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT>
	MoveNBackward(OutputIteratorT itOutputEnd,
		std::size_t uCount, InputIteratorT itInputEnd)
{
	return Impl_CopyMoveFill::CopyTraits<OutputIteratorT, InputIteratorT,
		std::decay_t<typename std::iterator_traits<InputIteratorT>::value_type>
		>::DoMoveNBackward(itOutputEnd, uCount, itInputEnd);
}

template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT
	ReverseCopy(OutputIteratorT itOutputBegin,
		std::common_type_t<InputIteratorT> itInputBegin, InputIteratorT itInputEnd)
{
	while(itInputBegin != itInputEnd){
		--itInputEnd;
		*itOutputBegin = *itInputEnd;
		++itOutputBegin;
	}
	return itOutputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT>
	ReverseCopyN(OutputIteratorT itOutputBegin,
		std::size_t uCount, InputIteratorT itInputEnd)
{
	for(auto i = uCount; i; --i){
		--itInputEnd;
		*itOutputBegin = *itInputEnd;
		++itOutputBegin;
	}
	return std::make_pair(itOutputBegin, itInputEnd);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT
	ReverseCopyBackward(OutputIteratorT itOutputEnd,
		InputIteratorT itInputBegin, std::common_type_t<InputIteratorT> itInputEnd)
{
	while(itInputBegin != itInputEnd){
		--itOutputEnd;
		*itOutputEnd = *itInputBegin;
		++itInputBegin;
	}
	return itOutputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT>
	ReverseCopyNBackward(OutputIteratorT itOutputEnd,
		InputIteratorT itInputBegin, std::size_t uCount)
{
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		*itOutputEnd = *itInputBegin;
		++itInputBegin;
	}
	return std::make_pair(itOutputEnd, itInputBegin);
}

template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT
	ReverseMove(OutputIteratorT itOutputBegin,
		std::common_type_t<InputIteratorT> itInputBegin, InputIteratorT itInputEnd)
{
	while(itInputBegin != itInputEnd){
		--itInputEnd;
		*itOutputBegin = std::move(*itInputEnd);
		++itOutputBegin;
	}
	return itOutputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT>
	ReverseMoveN(OutputIteratorT itOutputBegin,
		std::size_t uCount, InputIteratorT itInputEnd)
{
	for(auto i = uCount; i; --i){
		--itInputEnd;
		*itOutputBegin = std::move(*itInputEnd);
		++itOutputBegin;
	}
	return std::make_pair(itOutputBegin, itInputEnd);
}
template<typename OutputIteratorT, typename InputIteratorT>
inline OutputIteratorT
	ReverseMoveBackward(OutputIteratorT itOutputEnd,
		InputIteratorT itInputBegin, std::common_type_t<InputIteratorT> itInputEnd)
{
	while(itInputBegin != itInputEnd){
		--itOutputEnd;
		*itOutputEnd = std::move(*itInputBegin);
		++itInputBegin;
	}
	return itOutputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
inline std::pair<OutputIteratorT, InputIteratorT>
	ReverseMoveNBackward(OutputIteratorT itOutputEnd,
		InputIteratorT itInputBegin, std::size_t uCount)
{
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		*itOutputEnd = std::move(*itInputBegin);
		++itInputBegin;
	}
	return std::make_pair(itOutputEnd, itInputBegin);
}

template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
inline OutputIteratorT
	Fill(OutputIteratorT itOutputBegin, std::common_type_t<OutputIteratorT> itOutputEnd,
		const ParamsT &...vParams)
{
	while(itOutputBegin != itOutputEnd){
		*itOutputBegin = std::conditional_t<std::is_void<ValueT>::value,
			std::remove_reference_t<decltype(*itOutputBegin)>, ValueT>(vParams...);
		++itOutputBegin;
	}
	return itOutputBegin;
}
template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
inline OutputIteratorT
	FillN(OutputIteratorT itOutputBegin, std::size_t uCount,
		const ParamsT &...vParams)
{
	for(auto i = uCount; i; --i){
		*itOutputBegin = std::conditional_t<std::is_void<ValueT>::value,
			std::remove_reference_t<decltype(*itOutputBegin)>, ValueT>(vParams...);
		++itOutputBegin;
	}
	return itOutputBegin;
}
template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
inline OutputIteratorT
	FillBackward(std::common_type_t<OutputIteratorT> itOutputBegin, OutputIteratorT itOutputEnd,
		const ParamsT &...vParams)
{
	while(itOutputBegin != itOutputEnd){
		--itOutputEnd;
		*itOutputEnd = std::conditional_t<std::is_void<ValueT>::value,
			std::remove_reference_t<decltype(*itOutputEnd)>, ValueT>(vParams...);
	}
	return itOutputEnd;
}
template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
inline OutputIteratorT
	FillNBackward(std::size_t uCount, OutputIteratorT itOutputEnd,
		const ParamsT &...vParams)
{
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		*itOutputEnd = std::conditional_t<std::is_void<ValueT>::value,
			std::remove_reference_t<decltype(*itOutputEnd)>, ValueT>(vParams...);
	}
	return itOutputEnd;
}

}

#endif
