// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARGV_HPP_
#define MCF_CORE_ARGV_HPP_

#include <cstddef>
#include "../../MCFCRT/env/argv.h"
#include "../SmartPointers/UniquePtr.hpp"

namespace MCF {

struct ArgvResult {
	struct ArgItemDeleter {
		constexpr ::MCF_ArgItem *operator()() const noexcept {
			return nullptr;
		}
		void operator()(::MCF_ArgItem *pArgItem) const noexcept;
	};

	std::size_t uArgc;
	UniquePtr<const ::MCF_ArgItem [], ArgItemDeleter> pArgv;
};

extern ArgvResult GetArgv(const wchar_t *pwszCommandLine = nullptr);

}

#endif
