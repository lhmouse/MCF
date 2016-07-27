// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARGV_HPP_
#define MCF_CORE_ARGV_HPP_

#include <cstddef>
#include <MCFCRT/env/argv.h>
#include "../SmartPointers/UniquePtr.hpp"
#include "Noncopyable.hpp"
#include "Assert.hpp"
#include "StringView.hpp"

namespace MCF {

class Argv : MCF_NONCOPYABLE {
private:
	struct X_ArgItemDeleter {
		constexpr ::_MCFCRT_ArgItem *operator()() const noexcept {
			return nullptr;
		}
		void operator()(::_MCFCRT_ArgItem *pArgItem) const noexcept {
			::_MCFCRT_FreeArgv(pArgItem);
		}
	};

private:
	std::size_t x_uArgc;
	UniquePtr<const ::_MCFCRT_ArgItem [], X_ArgItemDeleter> x_pArgv;

public:
	// 如果传入空指针，就使用当前 GetCommandLineW() 的返回值。
	explicit Argv(const wchar_t *pwszCommandLine = nullptr);

public:
	std::size_t GetSize() const noexcept {
		return x_uArgc;
	}

	const wchar_t *GetStr(std::size_t uIndex) const {
		if(uIndex > x_uArgc){ // 传入 x_uArgc 会得到一个空指针。
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Argv: 下标越界。"));
		}
		return UncheckedGetStr(uIndex);
	}
	const wchar_t *UncheckedGetStr(std::size_t uIndex) const noexcept {
		MCF_ASSERT(uIndex <= x_uArgc);

		return x_pArgv[uIndex].__pwszStr;
	}

	std::size_t GetLen(std::size_t uIndex) const {
		if(uIndex > x_uArgc){ // 传入 x_uArgc 会得到 0。
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Argv: 下标越界。"));
		}
		return UncheckedGetLen(uIndex);
	}
	std::size_t UncheckedGetLen(std::size_t uIndex) const noexcept {
		MCF_ASSERT(uIndex <= x_uArgc);

		return x_pArgv[uIndex].__uLen;
	}

	WideStringView Get(std::size_t uIndex) const {
		if(uIndex > x_uArgc){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Argv: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	WideStringView UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_ASSERT(uIndex <= x_uArgc);

		return WideStringView(x_pArgv[uIndex].__pwszStr, x_pArgv[uIndex].__uLen);
	}

public:
	WideStringView operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
};

}

#endif
