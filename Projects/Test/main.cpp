#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <MCF/Serialization/Serdes.hpp>
#include <iostream>
using namespace std;
using namespace MCF;

extern "C" unsigned int MCFMain(){
	double d1, d2;
	StreamBuffer buf1, buf2;
	d1 = 123.456;
	buf1 << d1;
	buf2 << buf1;
	printf("serialized: ");
	buf2.Traverse(
		[](auto pby, auto cb){
			for(unsigned i = 0; i < cb; ++i){
				printf("%02hhX ", pby[i]);
			}
		}
	);
	putchar('\n');
	buf1.Clear();
	buf2 >> buf1;
	buf1 >> d2;
	printf("d2 = %f\n", d2);
	return 0;
}
