// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_DYNAMIC_LINK_LIBRARY_HPP_
#define MCF_CORE_DYNAMIC_LINK_LIBRARY_HPP_

#include <type_traits>
#include <cstdint>
#include "UniqueHandle.hpp"
#include "String.hpp"

namespace MCF {

class DynamicLinkLibrary {
private:
	struct xLibraryFreer {
		struct Impl;
		using Handle = Impl *;

		Handle operator()() noexcept;
		void operator()(Handle hDll) noexcept;
	};

public:
	using RawProc = std::intptr_t (__stdcall *)();

private:
	UniqueHandle<xLibraryFreer> x_hDll;

public:
	DynamicLinkLibrary() noexcept = default;
	DynamicLinkLibrary(DynamicLinkLibrary &&) noexcept = default;
	DynamicLinkLibrary &operator=(DynamicLinkLibrary &&) = default;

	explicit DynamicLinkLibrary(const wchar_t *pwszPath)
		: DynamicLinkLibrary()
	{
		Open(pwszPath);
	}
	explicit DynamicLinkLibrary(const WideString &wsPath)
		: DynamicLinkLibrary()
	{
		Open(wsPath);
	}

	DynamicLinkLibrary(const DynamicLinkLibrary &) = delete;
	DynamicLinkLibrary &operator=(const DynamicLinkLibrary &) = delete;

public:
	bool IsOpen() const noexcept;
	void Open(const wchar_t *pwszPath);
	void Open(const WideString &wsPath);
	bool OpenNoThrow(const wchar_t *pwszPath);
	bool OpenNoThrow(const WideString &wsPath);
	void Close() noexcept;

	const void *GetBaseAddress() const noexcept;
	RawProc RawGetProcAddress(const char *pszName);

	template<typename FunctionT>
	FunctionT *GetProcAddress(const char *pszName){
		static_assert(std::is_function<FunctionT>::value, "FunctionT shall be a function type");

		return reinterpret_cast<FunctionT *>(RawGetProcAddress(pszName));
	}

public:
	explicit operator bool() const noexcept {
		return IsOpen();
	}
};

}

#endif
