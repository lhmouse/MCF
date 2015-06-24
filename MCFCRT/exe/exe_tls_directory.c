// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../env/mcfwin.h"
#include "../env/thread.h"

#define DEF_SECTION(x)	__attribute__((__section__(x), __used__))

DEF_SECTION(".CRT$XL@") const PIMAGE_TLS_CALLBACK vCallbackAt	= &__MCF_CRT_TlsCallback;
DEF_SECTION(".CRT$XL_") const PIMAGE_TLS_CALLBACK vCallback_	= nullptr;

DWORD _tls_index = 0;

DEF_SECTION(".tls$@@@") const unsigned char _tls_start	= 0;
DEF_SECTION(".tls$___") const unsigned char _tls_end	= 0;

DEF_SECTION(".tls") const IMAGE_TLS_DIRECTORY _tls_used = {
	.StartAddressOfRawData	= (UINT_PTR)&_tls_start,
	.EndAddressOfRawData	= (UINT_PTR)&_tls_end,
	.AddressOfIndex			= (UINT_PTR)&_tls_index,
	.AddressOfCallBacks		= (UINT_PTR)&vCallbackAt,
	.SizeOfZeroFill			= 0,
	.Characteristics		= 0
};
