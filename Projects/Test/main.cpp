#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	StandardInputStream  sin;
	StandardOutputStream sout;
	StandardErrorStream  serr;

	sout.Put("Enter a password:", 17);
	sin.SetEchoing(false);
	NarrowString str;
	int c;
	while((c = sin.Get()) >= 0){
		str.Push(c);
	}
	sin.SetEchoing(true);
	sout.Put('\n');

	sout.Put("The string was ", 15);
	sout.Put(str.GetData(), str.GetSize());
	sout.Put('\n');

	serr.Put("Char codes were ", 16);
	for(auto p = str.GetBegin(); p != str.GetEnd(); ++p){
		char temp[20];
		unsigned len = (unsigned)std::sprintf(temp, "%02hhX ", *p);
		serr.Put(temp, len);
	}
	serr.Put('\n');

	return 0;
}
