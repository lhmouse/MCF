#include <MCF/StdMCF.hpp>
#include <MCF/StreamFilters/IsaacExFilters.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer text;
	text.Insert("hello world!", 12);

	IsaacExEncoder("key", 3).FilterInPlace(text);

	std::printf("encrypted: ");
	text.Traverse([](auto pby, auto cb){
		for(auto i = cb; i; --i){
			std::printf("%02hhX ", *(pby++));
		}
	});
	std::putchar('\n');

	IsaacExDecoder("key", 3).FilterInPlace(text);

	std::printf("decrypted: ");
	text.Traverse([](auto pby, auto cb){
		for(auto i = cb; i; --i){
			std::putchar(*(pby++));
		}
	});
	std::putchar('\n');


	return 0;
}
