#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ThreadLocal.hpp>

using namespace MCF;

template class ThreadLocal<int>;
template class ThreadLocal<long double>;

ThreadLocal<int>         tls_i;
ThreadLocal<long double> tls_ld;

extern "C" unsigned MCFCRT_Main(){
	tls_i.Set(12);
	tls_ld.Set(3.45);

	auto print = []{
		std::printf("Thread ID = %zu, tls_i = %d, tls_ld = %Lf\n", Thread::GetCurrentId(), tls_i.Get(), tls_ld.Get());
	};

	auto thread = Thread::Create([&]{
		tls_i.Set(67);
		tls_ld.Set(8.9);
		print();
	});
	thread->Wait();

	print();

	return 0;
}
