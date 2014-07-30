// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_FILE_SYSTEM_HPP_
#define MCFBUILD_FILE_SYSTEM_HPP_

#include "Model.hpp"
#include "../MCF/Containers/StreamBuffer.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/File.hpp"

namespace MCFBuild {

extern MCF::WideString GetFullPath(const MCF::WideString &wcsSrc);

extern bool GetFileContents(MCF::StreamBuffer &sbufData, const MCF::WideString &wcsPath, bool bThrowOnFailure);
extern void PutFileContents(const MCF::WideString &wcsPath, const MCF::StreamBuffer &sbufData);

extern bool GetFileSha256(Sha256 &vSha256, const MCF::WideString &wcsPath, bool bThrowOnFailure);

extern void CreateDirectory(const MCF::WideString &wcsPath);
extern void RemoveFile(const MCF::WideString &wcsPath);

}

#endif
