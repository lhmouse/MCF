#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Assert.hpp>

extern "C" unsigned _MCFCRT_Main(){
	MCF_ASSERT_MSG(false, L"aa");
	return 0;
}
