#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ThreadLocal.hpp>
using namespace MCF;

Mutex m;

struct foo {
	foo(){
		auto l = m.GetLock();
		std::printf("ctor!    tid = %zu, this = %p\n", Thread::GetCurrentId(), this);
	}
	foo(const foo &){
		auto l = m.GetLock();
		std::printf("cp ctor! tid = %zu, this = %p\n", Thread::GetCurrentId(), this);
	}
	foo(foo &&) noexcept {
		auto l = m.GetLock();
		std::printf("mv ctor! tid = %zu, this = %p\n", Thread::GetCurrentId(), this);
	}
	void bark(){
		auto l = m.GetLock();
		std::printf("bark!    tid = %zu, this = %p\n", Thread::GetCurrentId(), this);
	}
	~foo(){
		auto l = m.GetLock();
		std::printf("dtor!    tid = %zu, this = %p\n", Thread::GetCurrentId(), this);
	}
};

ThreadLocal<foo> t_p;

void thread_proc(){
	t_p->bark();
	t_p->bark();
	t_p->bark();
}

extern "C" unsigned int MCFMain() noexcept {
	auto p = Thread::Create(thread_proc);
	t_p->bark();
	t_p->bark();
	t_p->bark();
	p->Join();
	return 0;
}
