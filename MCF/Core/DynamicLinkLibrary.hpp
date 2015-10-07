// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_DYNAMIC_LINK_LIBRARY_HPP_
#define MCF_CORE_DYNAMIC_LINK_LIBRARY_HPP_

#include <type_traits>
#include <cstdint>
#include "UniqueHandle.hpp"

namespace MCF {

class DynamicLinkLibrary {
private:
	struct X_LibraryFreer {
		void *operator()() noexcept;
		void operator()(void *hDll) noexcept;
	};

public:
	using RawProc = std::intptr_t (__stdcall *)();

private:
	UniqueHandle<X_LibraryFreer> x_hDll;

public:
	DynamicLinkLibrary() noexcept = default;

	explicit DynamicLinkLibrary(const wchar_t *pwszPath);

public:
	bool IsOpen() const noexcept;
	void Open(const wchar_t *pwszPath);
	bool OpenNoThrow(const wchar_t *pwszPath);
	void Close() noexcept;

	const void *GetBaseAddress() const noexcept;
	RawProc RawGetProcAddress(const char *pszName);
	RawProc RawRequireProcAddress(const char *pszName);

	template<typename FunctionT>
	FunctionT *GetProcAddress(const char *pszName){
		static_assert(std::is_function<FunctionT>::value, "FunctionT shall be a function type");

		return reinterpret_cast<FunctionT *>(RawGetProcAddress(pszName));
	}
	template<typename FunctionT>
	FunctionT *RequireProcAddress(const char *pszName){
		static_assert(std::is_function<FunctionT>::value, "FunctionT shall be a function type");

		return reinterpret_cast<FunctionT *>(RawRequireProcAddress(pszName));
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
