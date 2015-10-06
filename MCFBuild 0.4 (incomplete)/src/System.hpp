// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2015, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_SYSTEM_HPP_
#define MCFBUILD_SYSTEM_HPP_

#include <MCF/Core/String.hpp>
#include <MCF/Containers/Vector.hpp>
#include <MCF/SmartPointers/IntrusivePtr.hpp>
#include <cstdint>

namespace MCFBuild {

struct System {
	static void Print(MCF::WideStringView wsoText, bool bInsertsNewLine = true, bool bToStdErr = false) noexcept;
	static unsigned Shell(MCF::WideString &wcsStdOut, MCF::WideString &wcsStdErr, MCF::WideStringView wsoCommand);
	static MCF::WideString NormalizePath(const wchar_t *pwcPath);

	static MCF::Vector<MCF::WideString> GetUtf8FileContents(const wchar_t *pwcPath);
	static void PutUtf8FileContents(const wchar_t *pwcPath, const MCF::Vector<MCF::WideString> &vecContents, bool bToAppend = false);

	// 目录名会以一个反斜杠结尾。
	static MCF::Vector<MCF::WideString> GetFileList(MCF::WideString wcsPath, bool bIncludesHidden);

private:
	System() = delete;
};

}

#endif
