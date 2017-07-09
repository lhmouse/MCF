#include <MCF/Core/Exception.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

extern "C" unsigned _MCFCRT_Main() noexcept {
	using namespace MCF;
	const auto err_s = MakeIntrusive<StandardErrorStream>();
	try {
		MCF_THROW(Exception, 123, Rcntws::View(L"Test exception"));
	} catch(std::exception &e){
		err_s->Put("std::exception caught: ", 23);
		err_s->Put(e.what(), std::strlen(e.what()));
		err_s->Put('\n');
	}
	return 0;
}
