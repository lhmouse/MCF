#include <MCF/StdMCF.hpp>
#include <MCF/Streams/BufferInputStream.hpp>
#include <MCF/Streams/BufferOutputStream.hpp>
#include <MCF/StreamFilters/Base64InputStreamFilter.hpp>
#include <MCF/StreamFilters/Base64OutputStreamFilter.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	constexpr char strs[][64] = {
		"aA==",
		"aGU=",
		"aGVs",
		"aGVsbA==",
		"aGVsbG8=",
		"aGVsbG8g",
		"aGVsbG8gdw==",
		"aGVsbG8gd28=",
		"aGVsbG8gd29y",
		"aGVsbG8gd29ybA==",
		"aGVsbG8gd29ybGQ=",
		"aGVsbG8gd29ybGQh",
	};

	for(unsigned i = 0; i < sizeof(strs) / sizeof(strs[0]); ++i){
		auto is = MakeIntrusive<BufferInputStream>();
		is.GetBuffer().Put(strs[i], std::strlen(strs[i]));
		auto bs = MakeIntrusive<Base64InputStreamFilter>(is);

		int c;
		while((c = bs->Get()) >= 0){
			std::putchar(c);
		}
		std::putchar('\n');
	}

	return 0;
}
