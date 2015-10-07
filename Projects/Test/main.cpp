#include <type_traits>
#include <cstdio>

extern "C" unsigned MCFMain(){
	std::printf("%d\n", std::is_same<int (__stdcall *)(), __stdcall int (*)()>::value);
	return 0;
}
