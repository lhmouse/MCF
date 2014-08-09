// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_CONSOLE_OUTPUT_HPP_
#define MCFBUILD_CONSOLE_OUTPUT_HPP_

#include "../MCF/Core/StringObserver.hpp"

namespace MCFBuild {

extern void Print(const MCF::WideStringObserver &wsoString) noexcept;
extern void PrintErr(const MCF::WideStringObserver &wsoString) noexcept;

extern void FormatPrint(const MCF::WideStringObserver &wsoString) noexcept;
extern void FormatPrintErr(const MCF::WideStringObserver &wsoString) noexcept;

}

#endif
