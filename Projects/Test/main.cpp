#include <cstring>

char s1[100], s2[100];

volatile auto p = std::memmove;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	std::memset(s1, 'a', sizeof(s1));
	std::memset(s2, 'b', sizeof(s2));
	(*p)(s1, s2, sizeof(s1));

	std::memset(s1, 'a', sizeof(s1));
	std::memset(s2, 'b', sizeof(s2));
	(*p)(s2, s1, sizeof(s1));

	return 0;
}
