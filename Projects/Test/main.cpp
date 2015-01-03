#include <MCF/StdMCF.hpp>
#include <MCF/Core/Transaction.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	try {
		Transaction t;
		t.Add([]{ std::puts("locking 1"); return true; }, []{ std::puts("committing 1"); }, []{ std::puts("unlocking 1"); });
		t.Add([]{ std::puts("locking 2"); return true; }, []{ std::puts("committing 2"); }, []{ std::puts("unlocking 2"); });
		t.Add([]{ throw 123; std::puts("locking 3"); return true; }, []{ std::puts("committing 3"); }, []{ std::puts("unlocking 3"); });
		t.Commit();
	} catch(int e){
		std::printf("exception: e = %d\n", e);
	}
	return 0;
}
