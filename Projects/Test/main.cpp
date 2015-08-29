#include <MCF/StdMCF.hpp>
#include <MCF/Core/Time.hpp>

void rep_memset(void *dst, int ch, size_t cb){
	__asm__ __volatile__(
		"mov ecx, eax ;"
		"shl ecx, 8 ;"
		"or eax, ecx ;"
		"mov ecx, eax ;"
		"shl ecx, 16 ;"
		"or eax, ecx ;"
#ifdef _WIN64
		"mov rcx, rax ;"
		"shl rcx, 32 ;"
		"or rax, rcx ;"
		"mov rcx, rdx ;"
		"shr rcx, 3 ;"
		"rep stosq ;"
		"mov rcx, rdx ;"
		"and rcx, 7 ;"
#else
		"mov ecx, edx ;"
		"shr ecx, 2 ;"
		"rep stosd ;"
		"mov ecx, edx ;"
		"and ecx, 3 ;"
#endif
		"rep stosb ;"
		: "+D"(dst)
		: "a"(ch), "d"(cb)
		: "cx"
	);
}

extern "C" unsigned MCFMain(){
	constexpr std::size_t clen = 0x10000 + 6;
	constexpr std::size_t repeat = 100000;

	std::unique_ptr<char []> ptr(new char[clen]);
	std::memset(ptr.get(), 'a', clen);
	double t1, t2;

	t1 = MCF::GetHiResMonoClock();
	for(unsigned i = 0; i < repeat; ++i){
		rep_memset(ptr.get(), 'b', clen);
	}
	t2 = MCF::GetHiResMonoClock();
	std::printf("rep_memset time = %f\n", t2 - t1);

	t1 = MCF::GetHiResMonoClock();
	for(unsigned i = 0; i < repeat; ++i){
		std::memset(ptr.get(), 'c', clen);
	}
	t2 = MCF::GetHiResMonoClock();
	std::printf("MCFCRT memset time = %f\n", t2 - t1);

	return 0;
}
