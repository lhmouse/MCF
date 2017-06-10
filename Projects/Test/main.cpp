#include <cstring>
#include <cstdio>

char str[100];
volatile auto cp = std::memmove;

extern "C" unsigned _MCFCRT_Main() noexcept {
	std::memcpy(str, "abcdefghijklmnopqrstuvwxyz", 27);
	cp(str, str + 7, 19);
	std::puts(str);
	std::memcpy(str, "abcdefghijklmnopqrstuvwxyz", 27);
	cp(str + 7, str, 19);
	std::puts(str);
	return 0;
}
