// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VARIANT_HPP_
#define MCF_CORE_VARIANT_HPP_

#include <new>
#include <tuple>
#include <cstddef>
#include "../Utilities/MinMax.hpp"

namespace MCF {

template<typename ...TypesT>
class Variant {
	static_assert(sizeof...(TypesT) > 0, "No type specified for Variant?");

private:
	using xTypes = std::tuple<TypesT...>;

private:
	alignas(Max(alignof(TypesT)...)) unsigned char x_abyBuffer[Max(sizeof(TypesT)...)];
	std::size_t x_uType;

public:
	Variant()
		: x_uType(0)
	{
		::new((void *)x_abyBuffer) std::tuple_element_t<0, xTypes>;
	}
};

}

#endif
