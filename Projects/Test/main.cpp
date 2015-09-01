#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/CopyMoveFill.hpp>
#include <MCF/Utilities/BinaryOperations.hpp>

extern "C" unsigned MCFMain(){
	static const std::initializer_list<volatile char> src = { 'a','b','c','d','e','f','g' };

	char dst[16] = { };
	const auto begin = std::begin(dst) + 1;
	const auto end = std::end(dst) - 2;

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out1 = MCF::Copy(begin, src.begin(), src.end());
	ASSERT(static_cast<std::size_t>(out1 - begin) == src.size());
	std::printf("str1  = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out2 = MCF::CopyBackward(end, src.begin(), src.end());
	ASSERT(static_cast<std::size_t>(end - out2) == src.size());
	std::printf("str2  = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out3 = MCF::CopyN(begin, src.begin(), src.size()).first;
	ASSERT(static_cast<std::size_t>(out3 - begin) == src.size());
	std::printf("str3  = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out4 = MCF::CopyNBackward(end, src.size(), src.end()).first;
	ASSERT(static_cast<std::size_t>(end - out4) == src.size());
	std::printf("str4  = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out5 = MCF::ReverseCopy(begin, src.begin(), src.end());
	ASSERT(static_cast<std::size_t>(out5 - begin) == src.size());
	std::printf("str5  = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out6 = MCF::ReverseCopyBackward(end, src.begin(), src.end());
	ASSERT(static_cast<std::size_t>(end - out6) == src.size());
	std::printf("str6  = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out7 = MCF::ReverseCopyN(begin, src.size(), src.end()).first;
	ASSERT(static_cast<std::size_t>(out7 - begin) == src.size());
	std::printf("str7  = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out8 = MCF::ReverseCopyNBackward(end, src.begin(), src.size()).first;
	ASSERT(static_cast<std::size_t>(end - out8) == src.size());
	std::printf("str8  = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out9 = MCF::Fill(begin, end, 'a');
	ASSERT(out9 == end);
	std::printf("str9  = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out10 = MCF::FillN(begin, src.size(), 'b');
	ASSERT(static_cast<std::size_t>(out10 - begin) == src.size());
	std::printf("str10 = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out11 = MCF::FillBackward(begin, end, 'c');
	ASSERT(out11 == begin);
	std::printf("str11 = %s\n", dst);

	std::memset(dst, '_', sizeof(dst) - 1);
	auto out12 = MCF::FillNBackward(src.size(), end, 'd');
	ASSERT(static_cast<std::size_t>(end - out12) == src.size());
	std::printf("str12 = %s\n", dst);

	return 0;
}
