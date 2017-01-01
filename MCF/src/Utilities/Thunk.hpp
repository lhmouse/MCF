// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_THUNK_HPP_
#define MCF_UTILITIES_THUNK_HPP_

#include "../SmartPointers/UniquePtr.hpp"
#include "../Core/Assert.hpp"
#include "../Core/Exception.hpp"
#include <type_traits>
#include <MCFCRT/ext/thunk.h>
#include <MCFCRT/env/last_error.h>

namespace MCF {

class Thunk {
private:
	struct X_ThunkDeleter {
		constexpr const void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(const void *pThunk) const noexcept {
			::_MCFCRT_DeallocateThunk(pThunk, true);
		}
	};

private:
	UniquePtr<const void, X_ThunkDeleter> x_pThunk;

public:
	Thunk(const void *pInit, std::size_t uSize){
		if(!x_pThunk.Reset(::_MCFCRT_AllocateThunk(pInit, uSize))){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_AllocateThunk() 失败。"));
		}
	}

public:
	const void *GetRaw() const noexcept {
		return x_pThunk.Get();
	}

	template<typename FunctionPointerT>
	FunctionPointerT Get() const noexcept {
		static_assert(std::is_pointer<FunctionPointerT>::value, "FunctionPointerT shall be a pointer type");

		return reinterpret_cast<FunctionPointerT>(GetRaw());
	}

	void Swap(Thunk &rhs) noexcept {
		using std::swap;
		swap(x_pThunk, rhs.x_pThunk);
	}

public:
	friend void swap(Thunk &lhs, Thunk &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
