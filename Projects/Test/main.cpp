#include <MCF/StdMCF.hpp>
#include <MCF/Core/Argv.hpp>

extern "C" unsigned MCFCRT_Main(){
	MCF::Argv argv;
	for(unsigned i = 0; i < argv.GetSize(); ++i){
		DWORD bytes_written;
		::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), argv.GetStr(i), argv.GetLen(i), &bytes_written, nullptr);
		::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, &bytes_written, nullptr);
	}
	return 0;
}
