#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Serialization/Serdes.hpp>
#include <iterator>
#include <vector>
#include <iostream>
using namespace std;
using namespace MCF;

template void MCF::Serialize(StreamBuffer &, const vector<int> &);
template void MCF::Serialize(StreamBuffer &, const vector<bool> &);

unsigned int MCFMain()
try {
	vector<bool> v, v2;
	generate_n(back_inserter(v), 1000, []{ return rand() % 2; });
	StreamBuffer sbuf;
	Serialize(sbuf, v);
	printf("serialized size %zu\n", sbuf.GetSize());
	auto sbuf2 = sbuf;

	cout <<hex;
	copy(sbuf.GetReadIterator(), sbuf.GetReadEnd(), ostream_iterator<unsigned int>(cout));
	cout <<endl;

	cout <<boolalpha;
	Deserialize(v2, sbuf2);
	cout <<(v == v2);
	cout <<endl;

	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->m_ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->m_pszFunction);
		printf("  line = %lu\n", p->m_ulLine);
		printf("  msg  = %s\n", AnsiString(WideString(p->m_pwszMessage.get())).GetCStr());
	}
	return 0;
}
