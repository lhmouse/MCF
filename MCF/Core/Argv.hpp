// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARGV_HPP_
#define MCF_CORE_ARGV_HPP_

#include <cstddef>
#include "../../MCFCRT/env/argv.h"
#include "../SmartPointers/UniquePtr.hpp"
#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include "StringView.hpp"

namespace MCF {

class Argv : NONCOPYABLE {
private:
	struct X_ArgItemDeleter {
		constexpr ::MCF_ArgItem *operator()() const noexcept {
			return nullptr;
		}
		void operator()(::MCF_ArgItem *pArgItem) const noexcept {
			::MCF_CRT_FreeArgv(pArgItem);
		}
	};

private:
	std::size_t x_uArgc;
	UniquePtr<const ::MCF_ArgItem [], X_ArgItemDeleter> x_pArgv;

public:
	// 如果传入空指针，就使用当前 GetCommandLineW() 的返回值。
	explicit Argv(const wchar_t *pwszCommandLine = nullptr);

public:
	std::size_t GetSize() const noexcept {
		return x_uArgc;
	}
	const wchar_t *GetStr(std::size_t uIndex) const noexcept {
		ASSERT(uIndex <= x_uArgc); // 传入 x_uArgc 会得到一个空指针。

		return x_pArgv[uIndex].pwszStr;
	}
	std::size_t GetLen(std::size_t uIndex) const noexcept {
		ASSERT(uIndex <= x_uArgc);

		return x_pArgv[uIndex].uLen;
	}
	WideStringView Get(std::size_t uIndex) const noexcept {
		return WideStringView(GetStr(uIndex), GetLen(uIndex));
	}

public:
	WideStringView operator[](std::size_t uIndex) const noexcept {
		return Get(uIndex);
	}
};

}

#endif
