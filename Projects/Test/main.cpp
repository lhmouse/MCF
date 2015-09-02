#include <MCF/StdMCF.hpp>

wchar_t s1[] = L"abcd";
wchar_t s2[] = L"abc";

extern "C" unsigned MCFMain(){
	std::printf("cmp = %d\n", std::wcscmp(s1, s2));
	std::printf("cmp = %d\n", std::wcscmp(s2, s1));
	std::printf("cmp = %d\n", std::wcsncmp(s1, s2, 3));
	std::printf("cmp = %d\n", std::wcsncmp(s1, s2, 4));
	std::printf("cmp = %d\n", std::wcsncmp(s1, s2, 10));
	return 0;
}
