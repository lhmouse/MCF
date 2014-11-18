// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_FILE_SYSTEM_HPP_
#define MCFBUILD_FILE_SYSTEM_HPP_

#include "../MCF/Core/StreamBuffer.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/File.hpp"
#include <array>

namespace MCFBuild {

using Sha256 = std::array<unsigned char, 32>;

extern MCF::WideString GetFullPath(const MCF::WideString &wsSrc);

extern bool GetFileContents(MCF::StreamBuffer &sbufData, const MCF::WideString &wsPath, bool bThrowOnFailure);
extern void PutFileContents(const MCF::WideString &wsPath, const MCF::StreamBuffer &sbufData);

extern bool GetFileSha256(Sha256 &vSha256, const MCF::WideString &wsPath, bool bThrowOnFailure);

extern void CreateDirectory(const MCF::WideString &wsPath);
extern void RemoveFile(const MCF::WideString &wsPath);

}

#endif
