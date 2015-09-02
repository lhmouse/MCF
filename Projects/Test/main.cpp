#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/CopyMoveFill.hpp>
#include <MCF/Utilities/BinaryOperations.hpp>

char src[] = { 'a','b','c','d','e','f','g' };

extern "C" unsigned MCFMain(){
	char dst[16] = { };
	const auto begin = std::begin(dst) + 1;
	const auto end = std::end(dst) - 2;

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out1 = MCF::Copy(begin, std::begin(src), std::end(src));
	ASSERT(static_cast<std::size_t>(out1 - begin) == sizeof(src));
	std::printf("str1  = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out2 = MCF::CopyBackward(end, std::begin(src), std::end(src));
	ASSERT(static_cast<std::size_t>(end - out2) == sizeof(src));
	std::printf("str2  = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out3 = MCF::CopyN(begin, std::begin(src), sizeof(src)).first;
	ASSERT(static_cast<std::size_t>(out3 - begin) == sizeof(src));
	std::printf("str3  = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out4 = MCF::CopyNBackward(end, sizeof(src), std::end(src)).first;
	ASSERT(static_cast<std::size_t>(end - out4) == sizeof(src));
	std::printf("str4  = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out5 = MCF::ReverseCopy(begin, std::begin(src), std::end(src));
	ASSERT(static_cast<std::size_t>(out5 - begin) == sizeof(src));
	std::printf("str5  = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out6 = MCF::ReverseCopyBackward(end, std::begin(src), std::end(src));
	ASSERT(static_cast<std::size_t>(end - out6) == sizeof(src));
	std::printf("str6  = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out7 = MCF::ReverseCopyN(begin, sizeof(src), std::end(src)).first;
	ASSERT(static_cast<std::size_t>(out7 - begin) == sizeof(src));
	std::printf("str7  = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out8 = MCF::ReverseCopyNBackward(end, std::begin(src), sizeof(src)).first;
	ASSERT(static_cast<std::size_t>(end - out8) == sizeof(src));
	std::printf("str8  = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out9 = MCF::Fill(begin, end, 'a');
	ASSERT(out9 == end);
	std::printf("str9  = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out10 = MCF::FillN(begin, sizeof(src), 'b');
	ASSERT(static_cast<std::size_t>(out10 - begin) == sizeof(src));
	std::printf("str10 = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out11 = MCF::FillBackward(begin, end, 'c');
	ASSERT(out11 == begin);
	std::printf("str11 = %s\n", dst);

	std::memset(const_cast<char *>(dst), '_', sizeof(dst) - 1);
	auto out12 = MCF::FillNBackward(sizeof(src), end, 'd');
	ASSERT(static_cast<std::size_t>(end - out12) == sizeof(src));
	std::printf("str12 = %s\n", dst);

	return 0;
}
