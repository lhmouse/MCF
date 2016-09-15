// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ALIGNED_STORAGE_HPP_
#define MCF_CORE_ALIGNED_STORAGE_HPP_

#include <type_traits>
#include <cstddef>

namespace MCF {

namespace Impl_AlignedStorage {
	template<std::size_t kFirstT, std::size_t kSecondT>
	struct BinaryMax
		: std::integral_constant<std::size_t, !(kFirstT < kSecondT) ? kFirstT : kSecondT>
	{
	};

	template<std::size_t kFirstT, std::size_t ...kRestT>
	struct Max
		: std::integral_constant<std::size_t, BinaryMax<kFirstT, Max<kRestT...>::value>::value>
	{
	};
	template<std::size_t kFirstT>
	struct Max<kFirstT>
		: std::integral_constant<std::size_t, kFirstT>
	{
	};
}

template<typename ...ElementsT>
struct __attribute__((__may_alias__)) AlignedStorage {
	static_assert((std::is_object<ElementsT>::value && ...), "Only object types are allowed.");

	alignas(Impl_AlignedStorage::Max<alignof(ElementsT)...>::value) char a[Impl_AlignedStorage::Max<sizeof(ElementsT)...>::value];
};

}

#endif
