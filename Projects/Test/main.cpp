#include <MCF/StdMCF.hpp>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	StandardInputStream sin;
	char str[20];
	auto len = sin.Get(str, sizeof(sin) - 1);
	str[len] = 0;
	std::printf("@ str = %s\n", str);

	StandardOutputStream sout;
	constexpr char out_str[] = "This line is printed to the standard output stream.\n";
	sout.Put(out_str, sizeof(out_str) - 1);

	StandardErrorStream serr;
	constexpr char err_str[] = "This line is printed to the standard error stream.\n";
	serr.Put(err_str, sizeof(err_str) - 1);

	return 0;
}
