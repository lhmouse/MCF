// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_FILE_SYSTEM_HPP_
#define MCFBUILD_FILE_SYSTEM_HPP_

#include "../MCF/Core/String.hpp"

namespace MCFBuild {

extern unsigned long long GetFileLastWriteTime(const MCF::WideString &wcsPath) noexcept;

extern MCF::WideString GetFullPath(const MCF::WideString &wcsSrc);

extern void CreateDirectory(const MCF::WideString &wcsPath);

}

#endif
