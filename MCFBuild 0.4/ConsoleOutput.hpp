// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_CONSOLE_OUTPUT_HPP_
#define MCFBUILD_CONSOLE_OUTPUT_HPP_

#include "../MCF/Core/StringObserver.hpp"

namespace MCFBuild {

extern void Print(const MCF::WideStringObserver &wsoString) noexcept;
extern void PrintLn(const MCF::WideStringObserver &wsoString = nullptr) noexcept;
extern void PrintErr(const MCF::WideStringObserver &wsoString) noexcept;

}

#endif
