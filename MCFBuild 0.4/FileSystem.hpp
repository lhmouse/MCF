// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_FILE_SYSTEM_HPP_
#define MCFBUILD_FILE_SYSTEM_HPP_

#include "Model.hpp"
#include "../MCF/Core/VVector.hpp"
#include "../MCF/Core/String.hpp"

namespace MCFBuild {

extern MCF::WideString GetFullPath(const MCF::WideString &wcsSrc);

extern bool GetFileContents(MCF::Vector<unsigned char> &vecData, const MCF::WideString &wcsPath);
extern void PutFileContents(const MCF::WideString &wcsPath, const void *pData, std::size_t uSize);
extern bool GetFileSha256(Sha256 &shaChecksum, const MCF::WideString &wcsPath);

extern void CreateDirectory(const MCF::WideString &wcsPath);
extern void RemoveFile(const MCF::WideString &wcsPath);

}

#endif
