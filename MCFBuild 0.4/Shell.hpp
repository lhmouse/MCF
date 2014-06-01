// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_SHELL_HPP_
#define MCFBUILD_SHELL_HPP_

#include "../MCF/Core/StringObserver.hpp"
#include "../MCF/Core/VVector.hpp"

namespace MCFBuild {

extern unsigned int Shell(
	MCF::Vector<char> &restrict vecStdOut,
	MCF::Vector<char> &restrict vecStdErr,
	const MCF::WideStringObserver &wsoCommandLine
);

}

#endif
