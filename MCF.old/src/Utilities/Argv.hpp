// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_ARGV_HPP_
#define MCF_UTILITIES_ARGV_HPP_

#include "../SmartPointers/UniquePtr.hpp"
#include "../Core/Assert.hpp"
#include "../Core/Exception.hpp"
#include "../Core/StringView.hpp"
#include <MCFCRT/env/argv.h>
#include <MCFCRT/env/last_error.h>

namespace MCF {

class Argv {
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
	explicit Argv(const wchar_t *pwszCommandLine = nullptr){
		if(pwszCommandLine){
			if(!x_pArgv.Reset(::_MCFCRT_AllocArgv(&x_uArgc, pwszCommandLine))){
				MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_AllocArgv() 失败。"));
			}
		} else {
			if(!x_pArgv.Reset(::_MCFCRT_AllocArgvFromCommandLine(&x_uArgc))){
				MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"_MCFCRT_AllocArgvFromCommandLine() 失败。"));
			}
		}
	}

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
		MCF_DEBUG_CHECK(uIndex <= x_uArgc);

		return x_pArgv[uIndex].__pwszStr;
	}

	std::size_t GetLen(std::size_t uIndex) const {
		if(uIndex > x_uArgc){ // 传入 x_uArgc 会得到 0。
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Argv: 下标越界。"));
		}
		return UncheckedGetLen(uIndex);
	}
	std::size_t UncheckedGetLen(std::size_t uIndex) const noexcept {
		MCF_DEBUG_CHECK(uIndex <= x_uArgc);

		return x_pArgv[uIndex].__uLen;
	}

	WideStringView Get(std::size_t uIndex) const {
		if(uIndex > x_uArgc){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Argv: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	WideStringView UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_DEBUG_CHECK(uIndex <= x_uArgc);

		return WideStringView(x_pArgv[uIndex].__pwszStr, x_pArgv[uIndex].__uLen);
	}

	void Swap(Argv &vOther) noexcept {
		using std::swap;
		swap(x_uArgc, vOther.x_uArgc);
		swap(x_pArgv, vOther.x_pArgv);
	}

public:
	WideStringView operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}

	friend void swap(Argv &vSelf, Argv &vOther) noexcept {
		vSelf.Swap(vOther);
	}
};

}

#endif
