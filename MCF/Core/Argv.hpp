// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARGV_HPP_
#define MCF_CORE_ARGV_HPP_

#include <memory>
#include <cstddef>
#include "../../MCFCRT/env/argv.h"

namespace MCF {

struct ArgvResult {
	struct ArgItemDeleter {
		void operator()(const ::MCF_ArgItem *pArgItem) const noexcept;
	};

	std::size_t uArgc;
	std::unique_ptr<const ::MCF_ArgItem [], ArgItemDeleter> pArgv;
};

extern ArgvResult GetArgv(const wchar_t *pwszCommandLine = nullptr);

}

#endif
