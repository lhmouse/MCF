#include <MCF/StdMCF.hpp>
#include <iostream>
#include <MCFCRT/env/contiguous_buffer.h>

/*
extern void _MCFCRT_ContiguousBufferPeek(const _MCFCRT_ContiguousBuffer *restrict __pBuffer, void **restrict __ppData, _MCFCRT_STD size_t *restrict __puSize) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_ContiguousBufferDiscard(_MCFCRT_ContiguousBuffer *restrict __pBuffer, _MCFCRT_STD size_t __uSizeToDiscard) _MCFCRT_NOEXCEPT;

extern bool _MCFCRT_ContiguousBufferReserve(_MCFCRT_ContiguousBuffer *restrict __pBuffer, void **restrict __ppData, _MCFCRT_STD size_t *restrict __puSizeReserved, _MCFCRT_STD size_t __uSizeToReserve) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_ContiguousBufferAdopt(_MCFCRT_ContiguousBuffer *restrict __pBuffer, _MCFCRT_STD size_t __uSizeToAdopt) _MCFCRT_NOEXCEPT;

extern void _MCFCRT_ContiguousBufferRecycle(_MCFCRT_ContiguousBuffer *restrict __pBuffer) _MCFCRT_NOEXCEPT;
*/

extern "C" unsigned _MCFCRT_Main(void){
	::_MCFCRT_ContiguousBuffer buf = { };

	constexpr char s[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	constexpr unsigned ns = sizeof(s) - 1;
	constexpr unsigned nr = 30;

	void *p;
	std::size_t n;

	for(unsigned i = 0; i < 10; ++i){
		::_MCFCRT_ContiguousBufferReserve(&buf, &p, &n, ns);
		std::cout <<"reserved: p = " <<p <<", n = " <<n <<std::endl;

		std::memcpy(p, s, ns);
		::_MCFCRT_ContiguousBufferAdopt(&buf, ns);
		std::cout <<"adopted: p = " <<p <<", ns = " <<ns <<std::endl;

		::_MCFCRT_ContiguousBufferPeek(&buf, &p, &n);
		std::cout <<"peeked: p = " <<p <<", n = " <<n <<std::endl;

		std::cout <<"> nr = " <<nr <<": ";
		std::cout.write((const char *)p, nr);
		std::cout <<std::endl;

		const unsigned nd = nr - i;
		::_MCFCRT_ContiguousBufferDiscard(&buf, nd);
		std::cout <<"discarded: p = " <<p <<", nd = " <<nd <<std::endl;
	}

	::_MCFCRT_ContiguousBufferRecycle(&buf);
	std::cout <<"recycled: " <<std::endl;

	return 0;
}
