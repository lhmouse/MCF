#include <MCF/StdMCF.hpp>
#include <MCFCRT/ext/stpcpy.h>
#include <MCFCRT/startup/exe_decl.h>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

struct foo : MCF::IntrusiveBase<foo> {
};
template class MCF::IntrusiveBase<foo>;
template class MCF::IntrusivePtr<foo>;
template class MCF::IntrusiveWeakPtr<foo>;

extern "C" unsigned _MCFCRT_Main() noexcept {
	char buff[10];
	auto w = buff;
	w = ::_MCFCRT_stppcpy(w, std::end(buff), "hello ");
	w = ::_MCFCRT_stppcpy(w, std::end(buff), "world!");
	std::puts(buff);
	return 0;
}
