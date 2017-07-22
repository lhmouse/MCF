#include <MCF/Core/StreamBuffer.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

extern "C" unsigned _MCFCRT_Main() noexcept {
	const auto os = MCF::MakeIntrusive<MCF::StandardErrorStream>();
	for(unsigned i = 0; i < 30; ++i){
		MCF::StreamBuffer buf;
		for(unsigned j = 0; j < 26; ++j){
			buf.Put(static_cast<unsigned char>('A' + j));
		}
		auto buf1 = buf;
		auto buf2 = buf1.CutOff(i);
		int c;

		os->Put("buf1 = ", 7);
		while((c = buf1.Unput()) >= 0){
			os->Put(static_cast<unsigned char>(c));
		}
		os->Put('\n');

		os->Put("buf2 = ", 7);
		while((c = buf2.Unput()) >= 0){
			os->Put(static_cast<unsigned char>(c));
		}
		os->Put('\n');

		os->Put("---\n", 4);
	}
	return 0;
}
