// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_DYNAMIC_LINK_LIBRARY_HPP_
#define MCF_CORE_DYNAMIC_LINK_LIBRARY_HPP_

#include <type_traits>
#include <cstdint>
#include "UniqueHandle.hpp"
#include "StringView.hpp"

namespace MCF {

class DynamicLinkLibrary {
private:
	struct X_DllUnloader {
		constexpr void *operator()() noexcept {
			return nullptr;
		}
		void operator()(void *hDll) noexcept;
	};

public:
	using RawProc = std::intptr_t (__stdcall *)();

private:
	UniqueHandle<X_DllUnloader> x_hDll;

public:
	DynamicLinkLibrary() noexcept = default;

	explicit DynamicLinkLibrary(const WideStringView &wsvPath);

public:
	bool IsOpen() const noexcept;
	void Open(const WideStringView &wsvPath);
	bool OpenNoThrow(const WideStringView &wsvPath);
	void Close() noexcept;

	const void *GetBaseAddress() const noexcept;
	RawProc RawGetProcAddress(const NarrowStringView &nsvName);
	RawProc RawRequireProcAddress(const NarrowStringView &nsvName);

	template<typename FunctionPointerT>
	FunctionPointerT GetProcAddress(const NarrowStringView &nsvName){
		static_assert(std::is_pointer<FunctionPointerT>::value, "FunctionPointerT shall be a pointer type");

		return reinterpret_cast<FunctionPointerT>(RawGetProcAddress(nsvName));
	}
	template<typename FunctionPointerT>
	FunctionPointerT RequireProcAddress(const NarrowStringView &nsvName){
		static_assert(std::is_pointer<FunctionPointerT>::value, "FunctionPointerT shall be a pointer type");

		return reinterpret_cast<FunctionPointerT>(RawRequireProcAddress(nsvName));
	}

	void Swap(DynamicLinkLibrary &rhs) noexcept {
		using std::swap;
		swap(x_hDll, rhs.x_hDll);
	}

public:
	explicit operator bool() const noexcept {
		return IsOpen();
	}
};

inline void swap(DynamicLinkLibrary &lhs, DynamicLinkLibrary &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
