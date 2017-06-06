// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_COPY_MOVE_FILL_HPP_
#define MCF_CORE_COPY_MOVE_FILL_HPP_

#include <iterator>
#include <type_traits>
#include <utility>
#include <cstddef>

namespace MCF {

template<typename OutputIteratorT, typename InputIteratorT>
OutputIteratorT Copy(OutputIteratorT itOutputBegin, InputIteratorT itInputBegin, std::common_type_t<InputIteratorT> itInputEnd){
	while(itInputBegin != itInputEnd){
		*itOutputBegin = *itInputBegin;
		++itOutputBegin;
		++itInputBegin;
	}
	return itOutputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
InputIteratorT CopyInto(OutputIteratorT itOutputBegin, std::common_type_t<OutputIteratorT> itOutputEnd, InputIteratorT itInputBegin){
	while(itOutputBegin != itOutputEnd){
		*itOutputBegin = *itInputBegin;
		++itOutputBegin;
		++itInputBegin;
	}
	return itInputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
std::pair<OutputIteratorT, InputIteratorT> CopyN(OutputIteratorT itOutputBegin, InputIteratorT itInputBegin, std::size_t uCount){
	for(auto i = uCount; i; --i){
		*itOutputBegin = *itInputBegin;
		++itOutputBegin;
		++itInputBegin;
	}
	return std::make_pair(itOutputBegin, itInputBegin);
}

template<typename OutputIteratorT, typename InputIteratorT>
OutputIteratorT CopyBackward(OutputIteratorT itOutputEnd, std::common_type_t<InputIteratorT> itInputBegin, InputIteratorT itInputEnd){
	while(itInputBegin != itInputEnd){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = *itInputEnd;
	}
	return itOutputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
InputIteratorT CopyIntoBackward(std::common_type_t<OutputIteratorT> itOutputBegin, OutputIteratorT itOutputEnd, InputIteratorT itInputEnd){
	while(itOutputBegin != itOutputEnd){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = *itInputEnd;
	}
	return itInputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
std::pair<OutputIteratorT, InputIteratorT> CopyNBackward(OutputIteratorT itOutputEnd, std::size_t uCount, InputIteratorT itInputEnd){
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = *itInputEnd;
	}
	return std::make_pair(itOutputEnd, itInputEnd);
}

template<typename OutputIteratorT, typename InputIteratorT>
OutputIteratorT Move(OutputIteratorT itOutputBegin, InputIteratorT itInputBegin, std::common_type_t<InputIteratorT> itInputEnd){
	while(itInputBegin != itInputEnd){
		*itOutputBegin = std::move(*itInputBegin);
		++itOutputBegin;
		++itInputBegin;
	}
	return itOutputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
InputIteratorT MoveInto(OutputIteratorT itOutputBegin, std::common_type_t<OutputIteratorT> itOutputEnd, InputIteratorT itInputBegin){
	while(itOutputBegin != itOutputEnd){
		*itOutputBegin = std::move(*itInputBegin);
		++itOutputBegin;
		++itInputBegin;
	}
	return itInputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
std::pair<OutputIteratorT, InputIteratorT> MoveN(OutputIteratorT itOutputBegin, InputIteratorT itInputBegin, std::size_t uCount){
	for(auto i = uCount; i; --i){
		*itOutputBegin = std::move(*itInputBegin);
		++itOutputBegin;
		++itInputBegin;
	}
	return std::make_pair(itOutputBegin, itInputBegin);
}

template<typename OutputIteratorT, typename InputIteratorT>
OutputIteratorT MoveBackward(OutputIteratorT itOutputEnd, std::common_type_t<InputIteratorT> itInputBegin, InputIteratorT itInputEnd){
	while(itInputBegin != itInputEnd){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = std::move(*itInputEnd);
	}
	return itOutputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
InputIteratorT MoveIntoBackward(std::common_type_t<OutputIteratorT> itOutputBegin, OutputIteratorT itOutputEnd, InputIteratorT itInputEnd){
	while(itOutputBegin != itOutputEnd){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = std::move(*itInputEnd);
	}
	return itInputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
std::pair<OutputIteratorT, InputIteratorT> MoveNBackward(OutputIteratorT itOutputEnd, std::size_t uCount, InputIteratorT itInputEnd){
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = std::move(*itInputEnd);
	}
	return std::make_pair(itOutputEnd, itInputEnd);
}

template<typename OutputIteratorT, typename InputIteratorT>
OutputIteratorT ReverseCopy(OutputIteratorT itOutputBegin, std::common_type_t<InputIteratorT> itInputBegin, InputIteratorT itInputEnd){
	while(itInputBegin != itInputEnd){
		--itInputEnd;
		*itOutputBegin = *itInputEnd;
		++itOutputBegin;
	}
	return itOutputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
InputIteratorT ReverseCopyInto(OutputIteratorT itOutputBegin, std::common_type_t<OutputIteratorT> itOutputEnd, InputIteratorT itInputEnd){
	while(itOutputBegin != itOutputEnd){
		--itInputEnd;
		*itOutputBegin = *itInputEnd;
		++itOutputBegin;
	}
	return itInputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
std::pair<OutputIteratorT, InputIteratorT> ReverseCopyN(OutputIteratorT itOutputBegin, std::size_t uCount, InputIteratorT itInputEnd){
	for(auto i = uCount; i; --i){
		--itInputEnd;
		*itOutputBegin = *itInputEnd;
		++itOutputBegin;
	}
	return std::make_pair(itOutputBegin, itInputEnd);
}

template<typename OutputIteratorT, typename InputIteratorT>
OutputIteratorT ReverseCopyBackward(OutputIteratorT itOutputEnd, InputIteratorT itInputBegin, std::common_type_t<InputIteratorT> itInputEnd){
	while(itInputBegin != itInputEnd){
		--itOutputEnd;
		*itOutputEnd = *itInputBegin;
		++itInputBegin;
	}
	return itOutputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
InputIteratorT ReverseCopyIntoBackward(std::common_type_t<OutputIteratorT> itOutputBegin, OutputIteratorT itOutputEnd, InputIteratorT itInputBegin){
	while(itOutputBegin != itOutputEnd){
		--itOutputEnd;
		*itOutputEnd = *itInputBegin;
		++itInputBegin;
	}
	return itInputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
std::pair<OutputIteratorT, InputIteratorT> ReverseCopyNBackward(OutputIteratorT itOutputEnd, InputIteratorT itInputBegin, std::size_t uCount){
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		*itOutputEnd = *itInputBegin;
		++itInputBegin;
	}
	return std::make_pair(itOutputEnd, itInputBegin);
}

template<typename OutputIteratorT, typename InputIteratorT>
OutputIteratorT ReverseMove(OutputIteratorT itOutputBegin, std::common_type_t<InputIteratorT> itInputBegin, InputIteratorT itInputEnd){
	while(itInputBegin != itInputEnd){
		--itInputEnd;
		*itOutputBegin = std::move(*itInputEnd);
		++itOutputBegin;
	}
	return itOutputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
InputIteratorT ReverseMoveInto(OutputIteratorT itOutputBegin, std::common_type_t<OutputIteratorT> itOutputEnd, InputIteratorT itInputEnd){
	while(itOutputBegin != itOutputEnd){
		--itInputEnd;
		*itOutputBegin = std::move(*itInputEnd);
		++itOutputBegin;
	}
	return itInputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
std::pair<OutputIteratorT, InputIteratorT> ReverseMoveN(OutputIteratorT itOutputBegin, std::size_t uCount, InputIteratorT itInputEnd){
	for(auto i = uCount; i; --i){
		--itInputEnd;
		*itOutputBegin = std::move(*itInputEnd);
		++itOutputBegin;
	}
	return std::make_pair(itOutputBegin, itInputEnd);
}

template<typename OutputIteratorT, typename InputIteratorT>
OutputIteratorT ReverseMoveBackward(OutputIteratorT itOutputEnd, InputIteratorT itInputBegin, std::common_type_t<InputIteratorT> itInputEnd){
	while(itInputBegin != itInputEnd){
		--itOutputEnd;
		*itOutputEnd = std::move(*itInputBegin);
		++itInputBegin;
	}
	return itOutputEnd;
}
template<typename OutputIteratorT, typename InputIteratorT>
InputIteratorT ReverseMoveIntoBackward(std::common_type_t<OutputIteratorT> itOutputBegin, OutputIteratorT itOutputEnd, InputIteratorT itInputBegin){
	while(itOutputBegin != itOutputEnd){
		--itOutputEnd;
		*itOutputEnd = std::move(*itInputBegin);
		++itInputBegin;
	}
	return itInputBegin;
}
template<typename OutputIteratorT, typename InputIteratorT>
std::pair<OutputIteratorT, InputIteratorT> ReverseMoveNBackward(OutputIteratorT itOutputEnd, InputIteratorT itInputBegin, std::size_t uCount){
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		*itOutputEnd = std::move(*itInputBegin);
		++itInputBegin;
	}
	return std::make_pair(itOutputEnd, itInputBegin);
}

template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
OutputIteratorT Fill(OutputIteratorT itOutputBegin, std::common_type_t<OutputIteratorT> itOutputEnd, const ParamsT &...vParams){
	while(itOutputBegin != itOutputEnd){
		*itOutputBegin = std::conditional_t<std::is_void<ValueT>::value, std::remove_reference_t<decltype(*itOutputBegin)>, ValueT>(vParams...);
		++itOutputBegin;
	}
	return itOutputBegin;
}
template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
OutputIteratorT FillN(OutputIteratorT itOutputBegin, std::size_t uCount, const ParamsT &...vParams){
	for(auto i = uCount; i; --i){
		*itOutputBegin = std::conditional_t<std::is_void<ValueT>::value, std::remove_reference_t<decltype(*itOutputBegin)>, ValueT>(vParams...);
		++itOutputBegin;
	}
	return itOutputBegin;
}
template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
OutputIteratorT FillBackward(std::common_type_t<OutputIteratorT> itOutputBegin, OutputIteratorT itOutputEnd, const ParamsT &...vParams){
	while(itOutputBegin != itOutputEnd){
		--itOutputEnd;
		*itOutputEnd = std::conditional_t<std::is_void<ValueT>::value, std::remove_reference_t<decltype(*itOutputEnd)>, ValueT>(vParams...);
	}
	return itOutputEnd;
}
template<typename ValueT = void, typename OutputIteratorT, typename ...ParamsT>
OutputIteratorT FillNBackward(std::size_t uCount, OutputIteratorT itOutputEnd, const ParamsT &...vParams){
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		*itOutputEnd = std::conditional_t<std::is_void<ValueT>::value, std::remove_reference_t<decltype(*itOutputEnd)>, ValueT>(vParams...);
	}
	return itOutputEnd;
}

}

#endif
