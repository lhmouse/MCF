// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CALL_ON_EACH_HPP_
#define MCF_CALL_ON_EACH_HPP_

#include <utility>

namespace MCF {

template<typename Function, typename First, typename ...Params>
Function &&CallOnEach(Function &&vFunction, First &&vFirst, Params &&... vParams){
	vFunction(std::forward<First>(vFirst));
	CallOnEach(vFunction, std::forward<Params>(vParams)...);
	return std::forward<Function>(vFunction);
}
template<typename Function>
Function &&CallOnEach(Function &&vFunction){
	return std::forward<Function>(vFunction);
}

template<typename Function, typename First, typename ...Params>
Function &&ReverseCallOnEach(Function &&vFunction, First &&vFirst, Params &&... vParams){
	ReverseCallOnEach(vFunction, std::forward<Params>(vParams)...);
	vFunction(std::forward<First>(vFirst));
	return std::forward<Function>(vFunction);
}
template<typename Function>
Function &&ReverseCallOnEach(Function &&vFunction){
	return std::forward<Function>(vFunction);
}

}

#endif
