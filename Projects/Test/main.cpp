#include <MCF/StdMCF.hpp>
#include <MCF/Containers/FlatMap.hpp>
#include <cstdio>

using namespace MCF;

struct foo {
	int a;

	explicit foo(int r)
		: a(r)
	{
		std::printf("foo(): a = %d\n", a);
	}
	foo(const foo &r)
		: a(r.a)
	{
		std::printf("foo(const foo &): a = %d\n", a);
	}
	foo(foo &&r) noexcept
		: a(std::exchange(r.a, -1))
	{
		std::printf("foo(foo &&): a = %d\n", a);
	}
	~foo(){
		if(a == -1){
			return;
		}
		std::printf("~foo(): a = %d\n", a);
	}
};

bool operator<(const foo &l, const foo &r) noexcept {
	std::printf("cmp(foo, foo) ");
	return l.a < r.a;
}
bool operator<(int l, const foo &r) noexcept {
	std::printf("cmp(int, foo) ");
	return l < r.a;
}
bool operator<(const foo &l, int r) noexcept {
	std::printf("cmp(foo, int) ");
	return l.a < r;
}

template class FlatMap<foo, int>;

extern "C" unsigned MCFCRT_Main(){
	FlatMap<foo, int> s;

	auto add = [&](int i){
		std::printf("--- adding: %d\n", i);
		s.Add(i, i);
	};

	add(1);
	add(5);
	add(3);
	add(2);
	add(6);
	add(0);
	add(4);

	auto find = [&](int i){
		std::printf("--- searching for: %d\n", i);
		auto it = s.GetMatch(i);
		std::printf(" -- found: %td\n", it - s.GetBegin());
	};

	for(int i = 0; i < 8; ++i){
		find(i);
	}

	return 0;
}
