#include <MCF/StdMCF.hpp>
#include <MCF/Core/AlignedStorage.hpp>
#include <MCF/Core/Exception.hpp>

// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VARIANT_HPP_
#define MCF_CORE_VARIANT_HPP_

//#include "AlignedStorage.hpp"
//#include "Exception.hpp"
#include <cstddef>

namespace MCF {

namespace Impl_Variant {
}

template<typename ...ElementsT>
class Variant {
private:
	AlignedStorage<1, ElementsT...> x_vStorage;
	int x_nActiveIndex;

public:
	Variant() noexcept
		: x_nActiveIndex(-1)
	{
	}
	Variant(const Variant &rhs) noexcept((std::is_nothrow_copy_constructible<ElementsT>::value && ...))
		: Variant()
	{
		// TODO
	}
	Variant(Variant &&rhs) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...))
		: Variant()
	{
		// TODO
	}
	Variant &operator=(const Variant &rhs) noexcept((std::is_nothrow_copy_constructible<ElementsT>::value && ...)) {
		// TODO
		return *this;
	}
	Variant &operator=(Variant &&rhs) noexcept((std::is_nothrow_move_constructible<ElementsT>::value && ...)) {
		// TODO
		return *this;
	}
	~Variant(){
		// TODO
	}

public:
	int GetTypeIndex() const noexcept {
		return x_nActiveIndex;
	}
};

}

#endif

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	return 0;
}
