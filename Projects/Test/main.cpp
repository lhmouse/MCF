#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ThreadLocal.hpp>
#include <cstdio>

using namespace MCF;

struct foo {
	int id;

	foo(int i)
		: id(i)
	{
		std::printf("foo::foo(int), id = %d\n", id);
	}
	foo(const foo &r)
		: id(r.id)
	{
		std::printf("foo::foo(const foo &), id = %d\n", id);
	}
	foo &operator=(const foo &r){
		int old = id;
		id = r.id;
		std::printf("foo &foo::operator=(const foo &r), odl = %d, id = %d\n", old, id);
		return *this;
	}
	~foo(){
		std::printf("foo::~foo(), id = %d\n", id);
	}
};

extern "C" unsigned int MCFMain() noexcept {
	ThreadLocal<foo> tls(1);

	auto thrd = Thread::Create([&]{
		*tls = 2;

		try {
			std::puts("about to throw...");
			throw std::out_of_range("test out_of_range in thread");
		} catch(std::exception &e){
			std::printf("exception caught: what = %s\n", e.what());
		}
	});
	thrd->Join();

	*tls = 3;

	try {
		std::puts("about to throw...");
		throw std::out_of_range("test out_of_range");
	} catch(std::exception &e){
		std::printf("exception caught: what = %s\n", e.what());
	}

	return 0;
}
