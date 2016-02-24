#include <MCF/StdMCF.hpp>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	StandardOutputStream sout;
	StandardInputStream sin;

	sout.Put("Enter a string: ", 16);
	char str[100];
	unsigned len;
	len = sin.Get(str, sizeof(str));
	sout.Put("The string was: ", 16);
	sout.Put(str, len);

	return 0;
}
