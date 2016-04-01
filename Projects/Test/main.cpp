#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Thread/ThreadLocal.hpp>

template class MCF::ThreadLocal<int>;
template class MCF::ThreadLocal<long double>;
template class MCF::ThreadLocal<MCF::AnsiString>;

extern "C" unsigned _MCFCRT_Main(){
	MCF::ThreadLocal<int>             ti;
	MCF::ThreadLocal<long double>     tf;
	MCF::ThreadLocal<MCF::AnsiString> ts;

	ti.Set(12345);
	ti.Set(23456);
	ti.Set(34567);

	tf.Set(1.234);
	tf.Set(23.45);
	tf.Set(345.6);

	ts.Set("abc");
	ts.Set("def");
	ts.Set("ghi");

	return 0;
}
