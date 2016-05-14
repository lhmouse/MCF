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
	MCF::IntrusivePtr<foo> p(new foo);
	p->ReserveWeak();
	return 0;
}
