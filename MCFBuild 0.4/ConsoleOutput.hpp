// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_CONSOLE_OUTPUT_HPP_
#define MCFBUILD_CONSOLE_OUTPUT_HPP_

#include "../MCF/Core/StringObserver.hpp"

namespace MCFBuild {

extern void PrintLn(const MCF::WideStringObserver &wsoString = nullptr) noexcept;
extern void PrintLnErr(const MCF::WideStringObserver &wsoString = nullptr) noexcept;

}

#endif
