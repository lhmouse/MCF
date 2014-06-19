#include <MCF/StdMCF.hpp>
#include <MCF/Core/Utilities.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <iostream>
using namespace MCF;

unsigned int MCFMain(){
	const auto str = "_123456789"_nso;
	StreamBuffer buf, buf2;

	for(auto i = 3u; i; --i){
		Copy(buf.GetWriteIterator(), str.GetBegin(), str.GetEnd());
		buf.Insert(str.GetBegin(), str.GetSize());
	}
	buf.CutOut(buf2, 19);

	Copy(std::ostream_iterator<char>(std::cout), buf2.GetReadIterator(), buf2.GetReadEnd());
	std::cout <<'$' <<std::endl;

	unsigned char tmp[1];
	while(buf.Extract(tmp, sizeof(tmp))){
		for(char ch : tmp){
			std::cout <<ch;
		}
	}
	std::cout <<'$' <<std::endl;

	return 0;
}
