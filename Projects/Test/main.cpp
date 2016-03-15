#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/Md5OutputStream.hpp>

extern "C" unsigned MCFCRT_Main(){
	constexpr char str[] = "123 is a baka!";
	MCF::Md5OutputStream s;
	s.Put(str, sizeof(str) - 1);
	const auto val = s.Finalize();

	MCF::NarrowString ln;
	for(auto b : val){
		char temp[8];
		int tlen = std::sprintf(temp, "%02hhx", b);
		ln.Append(temp, (unsigned)tlen);
	}
	std::printf("str = %s, md5 = %s\n", str, ln.GetStr());
	return 0;
}
