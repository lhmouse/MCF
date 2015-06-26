// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VARIANT_HPP_
#define MCF_CORE_VARIANT_HPP_

#include "../Utilities/ConstructDestruct.hpp"
#include <type_traits>
#include <tuple>
#include <utility>
#include <cstddef>

namespace MCF {

namespace Impl_Variant {
	template<typename ValueT>
	union RawStorage {
		ValueT v;
		char aby[sizeof(v)];

		RawStorage() noexcept {
		}
		~RawStorage(){
		}
	};
}

template<typename ...Types>
class Variant {
	static_assert(sizeof...(TypesT) > 0, "No type specified for Variant?");

private:
	using xContainer = std::tuple<Impl_Variant::RawStorage<Types>...>;

private:
	std::size_t x_uActiveType;
	xContainer x_tupContainer;

public:


public:

public:

};

}

#endif
