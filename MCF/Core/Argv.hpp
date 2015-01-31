// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARGV_HPP_
#define MCF_CORE_ARGV_HPP_

#include <cstddef>
#include "../../MCFCRT/env/argv.h"
#include "../Core/StringObserver.hpp"
#include "../SmartPointers/UniquePtr.hpp"
#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"

namespace MCF {

class Argv : NONCOPYABLE {
private:
	struct xArgItemDeleter {
		constexpr ::MCF_ArgItem *operator()() const noexcept {
			return nullptr;
		}
		void operator()(::MCF_ArgItem *pArgItem) const noexcept {
			::MCF_CRT_FreeArgv(pArgItem);
		}
	};

private:
	std::size_t xm_uArgc;
	UniquePtr<const ::MCF_ArgItem [], xArgItemDeleter> xm_pArgv;

public:
	explicit Argv(const wchar_t *pwszCommandLine = nullptr);

public:
	std::size_t GetSize() const noexcept {
		return xm_uArgc;
	}
	const wchar_t *GetStr(std::size_t uIndex) const noexcept {
		ASSERT(uIndex <= xm_uArgc); // 传入 xm_uArgc 会得到一个空指针。
		return xm_pArgv[uIndex].pwszStr;
	}
	std::size_t GetLen(std::size_t uIndex) const noexcept {
		ASSERT(uIndex <= xm_uArgc);
		return xm_pArgv[uIndex].uLen;
	}
	WideStringObserver Get(std::size_t uIndex) const noexcept {
		return WideStringObserver(GetStr(uIndex), GetLen(uIndex));
	}
};

}

#endif
