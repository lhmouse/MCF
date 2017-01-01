// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

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
	bool OpenNothrow(const WideStringView &wsvPath);
	void Close() noexcept;

	const void *GetBaseAddress() const noexcept;

	RawProc GetProcAddressRaw(const NarrowStringView &nsvName) const;
	RawProc RequireProcAddressRaw(const NarrowStringView &nsvName) const;
	RawProc GetProcAddressRaw(unsigned uOrdinal) const;
	RawProc RequireProcAddressRaw(unsigned uOrdinal) const;

	template<typename FunctionPointerT>
	FunctionPointerT GetProcAddress(const NarrowStringView &nsvName) const {
		static_assert(std::is_pointer<FunctionPointerT>::value, "FunctionPointerT shall be a pointer type");

		return reinterpret_cast<FunctionPointerT>(GetProcAddressRaw(nsvName));
	}
	template<typename FunctionPointerT>
	FunctionPointerT RequireProcAddress(const NarrowStringView &nsvName) const {
		static_assert(std::is_pointer<FunctionPointerT>::value, "FunctionPointerT shall be a pointer type");

		return reinterpret_cast<FunctionPointerT>(RequireProcAddressRaw(nsvName));
	}
	template<typename FunctionPointerT>
	FunctionPointerT GetProcAddress(unsigned uOrdinal) const {
		static_assert(std::is_pointer<FunctionPointerT>::value, "FunctionPointerT shall be a pointer type");

		return reinterpret_cast<FunctionPointerT>(GetProcAddressRaw(uOrdinal));
	}
	template<typename FunctionPointerT>
	FunctionPointerT RequireProcAddress(unsigned uOrdinal) const {
		static_assert(std::is_pointer<FunctionPointerT>::value, "FunctionPointerT shall be a pointer type");

		return reinterpret_cast<FunctionPointerT>(RequireProcAddressRaw(uOrdinal));
	}

	void Swap(DynamicLinkLibrary &rhs) noexcept {
		using std::swap;
		swap(x_hDll, rhs.x_hDll);
	}

public:
	explicit operator bool() const noexcept {
		return IsOpen();
	}

	friend void swap(DynamicLinkLibrary &lhs, DynamicLinkLibrary &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
