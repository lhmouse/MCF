// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_ABSORB_HPP_
#define MCF_UTILITIES_ABSORB_HPP_

#include <utility>

namespace MCF {

// Absorb(foo, 1, 2, 3); -> { foo(1); foo(2); foo(3); }
template<typename FunctionT>
FunctionT &&Absorb(FunctionT &&vFunction){
	return std::forward<FunctionT>(vFunction);
}
template<typename FunctionT, typename FirstT, typename ...RemainingT>
FunctionT &&Absorb(FunctionT &&vFunction, FirstT &&vFirst, RemainingT &&...vRemaining){
	std::forward<FunctionT>(vFunction)(std::forward<FirstT>(vFirst));
	Absorb(std::forward<FunctionT>(vFunction), std::forward<RemainingT>(vRemaining)...);
	return std::forward<FunctionT>(vFunction);
}

// ReverseAbsorb(foo, 1, 2, 3); -> { foo(3); foo(2); foo(1); }
template<typename FunctionT>
FunctionT &&ReverseAbsorb(FunctionT &&vFunction){
	return std::forward<FunctionT>(vFunction);
}
template<typename FunctionT, typename FirstT, typename ...RemainingT>
FunctionT &&ReverseAbsorb(FunctionT &&vFunction, FirstT &&vFirst, RemainingT &&...vRemaining){
	ReverseAbsorb(std::forward<FunctionT>(vFunction), std::forward<RemainingT>(vRemaining)...);
	std::forward<FunctionT>(vFunction)(std::forward<FirstT>(vFirst));
	return std::forward<FunctionT>(vFunction);
}

}

#endif
