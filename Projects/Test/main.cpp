#include <MCF/StdMCF.hpp>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/InputStreamIterator.hpp>
#include <MCF/Streams/OutputStreamIterator.hpp>
#include <MCF/Utilities/CopyMoveFill.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	StandardOutputStream sout;
	StandardInputStream sin;
	CopyN(OutputStreamIterator(sout), InputStreamIterator(sin), 10);

	return 0;
}
