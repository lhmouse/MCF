#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Containers/Deque.hpp>
using namespace MCF;

template class Deque<Utf8String>;

extern "C" unsigned int MCFMain() noexcept {
	Deque<Utf8String> q2{ "alpha"_u8s, "beta"_u8s, "gamma"_u8s, "delta"_u8s };
	q2.Pop();
	q2.Shift();
	auto q = q2;
	while(!q.IsEmpty()){
		auto &s = q.GetBack();
		std::printf("str = %s\n", s.GetStr());
		q.Pop();
	}
	return 0;
}
