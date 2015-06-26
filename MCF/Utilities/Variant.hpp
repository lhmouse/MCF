// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VARIANT_HPP_
#define MCF_CORE_VARIANT_HPP_

#include <type_traits>
#include <tuple>
#include <utility>
#include <cstddef>
#include "../Utilities/ConstructDestruct.hpp"

namespace MCF {

namespace Impl_Variant {
	template<typename ValueT>
	union RawStorage {
		ValueT v;
		char aby[1];

		RawStorage() noexcept {
		}
		~RawStorage(){
		}

		RawStorage(const RawStorage &) = delete;
		RawStorage &operator=(const RawStorage &) = delete;
	};
}

template<typename ...TypesT>
class Variant {
	static_assert(sizeof...(TypesT) > 0, "No type specified for Variant?");

private:
	using xContainer = std::tuple<Impl_Variant::RawStorage<TypesT>...>;

private:
	std::size_t x_uActiveType;
	xContainer x_tupContainer;

private:

public:


public:
	std::size_t GetType() const noexcept {
		return x_uActiveType;
	}
	template<std::size_t kIndex>
	const std::tuple_element_t<kIndex, xContainer> *Get() const noexcept {
		if(x_uActiveType != kIndex){
			return nullptr;
		}
		return reinterpret_cast<const std::tuple_element_t<kIndex, xContainer> *>(static_cast<const char *>(std::get<kIndex>(x_tupContainer).aby));
	}
	template<std::size_t kIndex>
	std::tuple_element_t<kIndex, xContainer> *Get() noexcept {
		if(x_uActiveType != kIndex){
			return nullptr;
		}
		return reinterpret_cast<std::tuple_element_t<kIndex, xContainer> *>(static_cast<char *>(std::get<kIndex>(x_tupContainer).aby));
	}
	template<std::size_t kIndex, typename ...ParamsT>
	void Set(ParamsT &&...vParams){
	}

public:

};

}

#endif
