#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <MCF/Containers/List.hpp>

using namespace MCF;

template class List<int>;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	for(std::size_t i = 0; i < 30; ++i){
		StreamBuffer b1, b2;
		b1.Put("abcdefghijklmnopqrstuvexyz", 26);
		b2 = b1.CutOff(i);

		char str[64];
		std::size_t len;
		int c;

		len = 0;
		while((c = b1.Unput()) >= 0){
			str[len++] = (char)c;
		}
		str[len++] = '$';
		str[len++] = 0;
		std::printf("b1 (%2u) : %s\n", len, str);

		len = 0;
		while((c = b2.Unput()) >= 0){
			str[len++] = (char)c;
		}
		str[len++] = '$';
		str[len++] = 0;
		std::printf("b2 (%2u) : %s\n", len, str);

		std::puts("-----");
	}
	return 0;
}
