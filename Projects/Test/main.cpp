#define ZLIB_CONST
#include <MCFCRT/MCFCRT.h>
#include <External/zlib/zlib.h>
#include <cstdio>
#include <cstring>
#include <cstddef>

unsigned int MCFMain(){
	const unsigned char *src = (const unsigned char *)"Hello world!";
	unsigned char dst[256];
	const size_t len = std::strlen((const char *)src);

	std::printf("src = %s$\n", src);

	unsigned char compressed[256];
	size_t compressed_len;

	z_stream ctx;
	ctx.zalloc = Z_NULL;
	ctx.zfree = Z_NULL;
	ctx.opaque = Z_NULL;
	::deflateInit2(&ctx, 9, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY);

	ctx.next_in = src;
	ctx.avail_in = len;
	ctx.next_out = compressed;
	ctx.avail_out = sizeof(compressed);
	::deflate(&ctx, Z_FINISH);

	::deflateEnd(&ctx);
	compressed_len = ctx.total_out;

	std::printf("compressed len = %u\n", (unsigned int)compressed_len);

	ctx.next_in = compressed;
	ctx.avail_in = compressed_len;
	ctx.zalloc = Z_NULL;
	ctx.zfree = Z_NULL;
	ctx.opaque = Z_NULL;
	::inflateInit2(&ctx, -15);

	ctx.next_out = dst;
	ctx.avail_out = sizeof(dst);
	::inflate(&ctx, Z_FINISH);

	::inflateEnd(&ctx);
	dst[ctx.total_out] = 0;

	std::printf("dst = %s$\n", dst);

	return 0;
}
