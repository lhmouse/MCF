#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>

extern "C" unsigned MCFMain(){
	try {
		DEBUG_THROW(MCF::Exception, 100, "this is a test message");
	} catch(MCF::Exception &e){
		std::printf("code = %lu, message = %s\n", e.GetCode(), e.GetMessage());
	}
	return 0;
}
