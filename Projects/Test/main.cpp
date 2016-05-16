#include <MCF/StdMCF.hpp>
#include <MCFCRT/ext/stpcpy.h>
#include <MCFCRT/startup/exe_decl.h>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

#include <MCF/Core/String.hpp>
#include <MCF/Utilities/Invoke.hpp>

extern "C" unsigned _MCFCRT_Main() noexcept {
	MCF::NarrowString s;
	void (MCF::NarrowString::*p)(const char *) = &MCF::NarrowString::Append;
	MCF::Invoke(p, s, "hello ");
	MCF::Invoke(p, s, "world!");
	std::printf("s = %s\n", s.GetStr());
	return 0;
}
