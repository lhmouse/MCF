// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CALL_ON_EACH_HPP_
#define MCF_CALL_ON_EACH_HPP_

#include <utility>

namespace MCF {

template<typename FunctionT, typename FirstT, typename ...ParamsT>
FunctionT &&CallOnEach(FunctionT &&vFunction, FirstT &&vFirst, ParamsT &&...vParams){
	vFunction(std::forward<FirstT>(vFirst));
	CallOnEach(vFunction, std::forward<ParamsT>(vParams)...);
	return std::forward<FunctionT>(vFunction);
}
template<typename FunctionT>
FunctionT &&CallOnEach(FunctionT &&vFunction){
	return std::forward<FunctionT>(vFunction);
}

template<typename FunctionT, typename FirstT, typename ...ParamsT>
FunctionT &&ReverseCallOnEach(FunctionT &&vFunction, FirstT &&vFirst, ParamsT &&...vParams){
	ReverseCallOnEach(vFunction, std::forward<ParamsT>(vParams)...);
	vFunction(std::forward<FirstT>(vFirst));
	return std::forward<FunctionT>(vFunction);
}
template<typename FunctionT>
FunctionT &&ReverseCallOnEach(FunctionT &&vFunction){
	return std::forward<FunctionT>(vFunction);
}

}

#endif
