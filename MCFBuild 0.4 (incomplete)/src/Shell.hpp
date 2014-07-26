// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_SHELL_HPP_
#define MCFBUILD_SHELL_HPP_

#include "../MCF/Core/String.hpp"

namespace MCFBuild {

extern unsigned int Shell(
	MCF::WideString &restrict wcsStdOut, MCF::WideString &restrict wcsStdErr,
	const MCF::WideStringObserver &wsoCommandLine
);

}

#endif
