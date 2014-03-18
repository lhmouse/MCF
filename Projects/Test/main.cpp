#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCFCRT/cpp/ext/multi_indexed_map.hpp>
using namespace MCF;

unsigned int MCFMain(){
	MultiIndexedMap<int, UTF8String> mim;

	mim.Insert(1, "1");
	mim.Insert(2, "4");
	mim.Insert(3, "9");
	mim.Insert(4, "16");

	const char *const str = "16";
	const auto p = mim.Find<0>(str);
	std::printf("search for %s, result is %p\n", str, p);
	if(p){
		std::printf("  val = %d\n", p->GetElement());
	}

	return 0;
}
