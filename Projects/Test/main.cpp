#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Monitor.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

void my_print(const AnsiString &s){
	std::puts(s.GetCStr());
}

extern "C" unsigned int MCFMain() noexcept {
	Monitor<AnsiString> mon;
	mon->Assign("meow", 4);
	my_print(*mon);
	return 0;
}
