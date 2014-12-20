// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_TRAITS_HPP_
#define MCF_SMART_POINTERS_TRAITS_HPP_

#include "../Utilities/Assert.hpp"
#include <type_traits>
#include <cstddef>

namespace MCF {

namespace Impl {
	template<typename BaseT, typename ObjectT>
	struct SmartPointerCheckDereferencable {
		decltype(auto) operator*() const noexcept {
			ASSERT(static_cast<const BaseT *>(this)->IsNonnull());

			return static_cast<const BaseT *>(this)->Get()[0];
		}
		decltype(auto) operator->() const noexcept {
			ASSERT(static_cast<const BaseT *>(this)->IsNonnull());

			return static_cast<const BaseT *>(this)->Get();
		}
	};
	template<typename BaseT>
	struct SmartPointerCheckDereferencable<BaseT, const volatile void> {
	};
	template<typename BaseT>
	struct SmartPointerCheckDereferencable<BaseT, const void> {
	};
	template<typename BaseT>
	struct SmartPointerCheckDereferencable<BaseT, volatile void> {
	};
	template<typename BaseT>
	struct SmartPointerCheckDereferencable<BaseT, void> {
	};
	template<typename BaseT, typename ObjectT>
	struct SmartPointerCheckDereferencable<BaseT, ObjectT []> {
	};

	template<typename BaseT, typename ObjectT>
	struct SmartPointerCheckArray {
	};
	template<typename BaseT, typename ObjectT>
	struct SmartPointerCheckArray<BaseT, ObjectT []> {
		decltype(auto) operator[](std::size_t uIndex) const noexcept {
			ASSERT(static_cast<const BaseT *>(this)->IsNonnull());

			return static_cast<const BaseT *>(this)->Get()[uIndex];
		}
	};
}

}

#endif
