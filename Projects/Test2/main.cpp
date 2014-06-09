#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <iostream>
using namespace std;
using namespace MCF;

unsigned int MCFMain(){
	StreamBuffer buf, buf2;
	buf.Insert("hello world!", 12);
	cout <<buf.CutOut(buf2, 100) <<endl;

	cout <<buf.CutOut(buf2, 5) <<endl;
	Copy(ostream_iterator<char>(cout), buf2.GetReadIterator(), buf2.GetReadEnd());
	cout <<endl;
	Copy(ostream_iterator<char>(cout), buf.GetReadIterator(), buf.GetReadEnd());
	cout <<endl;
	return 0;
}
