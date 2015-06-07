// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VARIANT_HPP_
#define MCF_CORE_VARIANT_HPP_

#include "../Utilities/ParameterPackManipulators.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include <type_traits>
#include <utility>
#include <cstddef>
/*
namespace MCF {

namespace Impl_Variant {
	template<std::size_t FIRST_T>
	constexpr std::size_t Max() noexcept {
		return FIRST_T;
	}
	template<std::size_t FIRST_T, std::size_t SECOND_T, std::size_t ...REMAINING_T>
	constexpr std::size_t Max() noexcept {
		return Max<(FIRST_T >= SECOND_T) ? FIRST_T : SECOND_T, REMAINING_T...>();
	}
}

template<typename ...TypesT>
class Variant {
	static_assert(sizeof...(TypesT) > 0, "No type specified for Variant?");

private:
	alignas(Impl_Variant::Max<alignof(TypesT)...>())
		unsigned char x_abyBuffer[Impl_Variant::Max<sizeof(TypesT)...>()];
	std::size_t x_uType;

public:
	template<typename T>
	explicit Variant(T &&vInit){
		using InitType = std::decay_t<T>;

		static_assert(FindFirstType<InitType, TypesT...>() == FindLastType<InitType, TypesT...>(), "Duplicate type specified in Variant?");

		Construct<InitType>(reinterpret_cast<InitType *>(x_abyBuffer), std::forward<T>(vInit));
		x_uType = FindFirstType<InitType, TypesT...>();
	}
	Variant()
		: Variant(NthType<0, TypesT...>())
	{
	}

public:
	template<std::size_t INDEX_T>
	auto GetByIndex() const noexcept {
		return (x_uType == INDEX_T) ? reinterpret_cast<const NthType<INDEX_T, TypesT...> *>(x_abyBuffer) : nullptr;
	}
	template<std::size_t INDEX_T>
	auto GetByIndex() noexcept {
		return (x_uType == INDEX_T) ? reinterpret_cast<NthType<INDEX_T, TypesT...> *>(x_abyBuffer) : nullptr;
	}

	template<typename T>
	auto GetByType() const noexcept {
		return (x_uType == FindFirstType<std::decay_t<T>, TypesT...>()) ? reinterpret_cast<const T *>(x_abyBuffer) : nullptr;
	}
	template<typename T>
	auto GetByType() noexcept {
		return (x_uType == FindFirstType<std::decay_t<T>, TypesT...>()) ? reinterpret_cast<T *>(x_abyBuffer) : nullptr;
	}

	std::size_t GetType() const noexcept {
		return x_uType;
	}


};

}
*/
#endif
