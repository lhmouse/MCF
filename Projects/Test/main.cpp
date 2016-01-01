#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/TupleManipulators.hpp>

using namespace MCF;

struct probe {
	int id;

	probe()
		: id(-1)
	{
	}
	probe(int i)
		: id(i)
	{
		std::printf("probe(%d) constructed\n", id);
	}
	probe(const probe &r) noexcept
		: id(r.id)
	{
		std::printf("probe(%d) copied\n", id);
	}
	probe(probe &&r) noexcept
		: id(std::exchange(r.id, -1))
	{
		std::printf("probe(%d) moved\n", id);
	}
	~probe() noexcept {
		if(id == -1){
			return;
		}
		std::printf("probe(%d) destroyed\n", id);
	}
};

extern "C" unsigned MCFMain(){
	auto t = std::make_tuple(probe(1), probe(2), probe(3));
	std::puts("----------");

	auto af = [](auto &&p){ std::printf("absorbed probe(%d)\n", std::exchange(p.id, -1)); };
	AbsorbTuple(af, t);
	std::puts("----------");
	ReverseAbsorbTuple(af, t);
	std::puts("----------");

	auto sf = [](auto &&p1, auto &&p2, auto &&p3){ std::printf("squeezed probe(%d), probe(%d), probe(%d)\n", std::exchange(p1.id, -1), std::exchange(p2.id, -1), std::exchange(p3.id, -1)); };
	SqueezeTuple(sf, t);
	std::puts("----------");
	ReverseSqueezeTuple(sf, t);
	std::puts("----------");

	return 0;
}
