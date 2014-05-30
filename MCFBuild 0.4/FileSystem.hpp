// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_FILE_SYSTEM_HPP_
#define MCFBUILD_FILE_SYSTEM_HPP_

#include "Model.hpp"
#include "../MCF/Core/String.hpp"

namespace MCFBuild {

extern bool GetFileSha256(Sha256 &shaRet, const MCF::WideString &wcsPath);

extern MCF::WideString GetFullPath(const MCF::WideString &wcsSrc);

extern void CreateDirectory(const MCF::WideString &wcsPath);

}

#endif
