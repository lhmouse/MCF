#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ReaderWriterLock.hpp>
using namespace MCF;

unsigned int MCFMain(){
	auto rwl = ReaderWriterLock::Create();

	auto lock1 = rwl->GetReaderLock();
	auto lock2 = rwl->GetWriterLock();

	return 0;
}
