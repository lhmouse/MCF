#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
using namespace std;
using namespace MCF;

unsigned int MCFMain(){
	static const char data[10000] = { };

	StreamBuffer buf;
	buf.Insert(data, sizeof(data));
	buf.Insert(data, sizeof(data));
	buf.Insert(data, sizeof(data));

	return 0;
}
