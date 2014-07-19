#include <MCF/StdMCF.hpp>
#include <MCF/Core/File.hpp>
#include <MCF/Hash/Md5.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	static constexpr std::size_t FILE_BUFFER_SIZE = 0x10000;

	const auto pFile = File::Open(L"E:\\big.bin"_wso, File::TO_READ);
	const auto u64FileSize = pFile->GetSize();
	Md5 md5Hasher;
	md5Hasher.Update(nullptr, 0);
	if(u64FileSize > 0){
		unsigned char abyTemp1[FILE_BUFFER_SIZE], abyTemp2[FILE_BUFFER_SIZE];
		auto *pbyCurBuffer = abyTemp1, *pbyBackBuffer = abyTemp2;
		std::size_t uBytesCur = pFile->Read(pbyCurBuffer, FILE_BUFFER_SIZE, 0);

		std::uint64_t u64Offset = uBytesCur;
		while(u64Offset < u64FileSize){
			const auto uBytesBack = pFile->Read(
				pbyBackBuffer, FILE_BUFFER_SIZE, u64Offset,
				[&]{
					md5Hasher.Update(pbyCurBuffer, uBytesCur);
				}
			);
			u64Offset += uBytesBack;

			std::swap(pbyCurBuffer, pbyBackBuffer);
			uBytesCur = uBytesBack;
		}

		md5Hasher.Update(pbyCurBuffer, uBytesCur);
	}
	unsigned char abyMd5[16];
	md5Hasher.Finalize(abyMd5);
	for(auto by : abyMd5){
		std::printf("%02hhX", by);
	}
	std::putchar('\n');
	return 0;
}
