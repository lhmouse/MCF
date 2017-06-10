#include <cstring>
#include <cstdio>

char str[100];

extern "C" unsigned _MCFCRT_Main() noexcept {
	std::memcpy(str, "abcdefghijklmnopqrstuvwxyz", 27);
	std::memmove(str, str + 1, 10);
	std::puts(str);
	std::memcpy(str, "abcdefghijklmnopqrstuvwxyz", 27);
	std::memmove(str + 1, str, 10);
	std::puts(str);
	return 0;
}
