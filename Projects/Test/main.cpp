#include <MCF/StdMCF.hpp>
#include <MCF/Core/Time.hpp>

using namespace MCF;

std::size_t repnz_strlen(const char *ptr){
	std::size_t ret;
	__asm__ __volatile__(
		"mov rdi, rax ;"
		"xor rax, rax ;"
		"xor rcx, rcx ;"
		"dec rcx ;"
		"repnz scasb ;"
		"not rcx ;"
		"dec rcx ;"
		: "=c"(ret)
		: "a"(ptr)
		: "rdi"
	);
	return ret;
}

extern "C" unsigned MCFMain(){
	constexpr std::size_t clen = 0x100000000;

	std::unique_ptr<char []> ptr(new char[clen]);
	std::memset(ptr.get(), 'a', clen);
	ptr[clen - 1] = 0;

	auto t1 = GetHiResMonoClock();
	auto len = repnz_strlen(ptr.get());
	auto t2 = GetHiResMonoClock();
	std::printf("repnz_strlen time = %f, len = %zX\n", t2 - t1, len);

	t1 = GetHiResMonoClock();
	len = std::strlen(ptr.get());
	t2 = GetHiResMonoClock();
	std::printf("MCFCRT strlen time = %f, len = %zX\n", t2 - t1, len);

	return 0;
}
