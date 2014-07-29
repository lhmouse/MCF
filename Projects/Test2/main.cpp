#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/File.hpp>
#include <MCF/StreamFilters/LzmaFilters.hpp>
#include <MCF/Hash/Crc32.hpp>
#include <iostream>
#include <iomanip>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	/*try {
		const auto pInputFile = File::Open(L"E:\\large.txt"_wso, File::TO_READ);
		const auto uInputSize = pInputFile->GetSize();
		StreamBuffer sbufData;
		while(sbufData.GetSize() < uInputSize){
			unsigned char abyTemp[0x1000];
			const auto uRead = pInputFile->Read(abyTemp, sizeof(abyTemp), sbufData.GetSize());
			sbufData.Insert(abyTemp, uRead);
		}

		Crc32 vCrc;
		sbufData.Traverse([&](auto p, auto cb){ vCrc.Update(p, cb); });
		std::printf("original size = %zu bytes, crc = %08lX\n", sbufData.GetSize(), (unsigned long)vCrc.Finalize());

		LzmaEncoder::Create()->FilterInPlace(sbufData);
		std::printf("compressed size = %zu bytes\n", sbufData.GetSize());

		LzmaDecoder::Create()->FilterInPlace(sbufData);
		sbufData.Traverse([&](auto p, auto cb){ vCrc.Update(p, cb); });
		std::printf("decompressed size = %zu bytes, crc = %08lX\n", sbufData.GetSize(), (unsigned long)vCrc.Finalize());
	} catch(Exception &e){
		::MessageBoxW(0, e.m_wcsMessage.GetCStr(), 0, 0);
	}*/

	const StreamBuffer buf("1234567890123456789012345678901234567890", 40);
	for(std::size_t i = 0; i < 45; ++i){
		std::cout <<std::setw(3) <<i <<": ";
		auto buf2 = buf;
		StreamBuffer buf3;
		buf2.CutOut(buf3, i);
		std::cout <<std::setw(3) <<buf3.GetSize() <<": ";
		Copy(std::ostream_iterator<unsigned char>(std::cout), buf3.GetReadIterator(), buf3.GetReadEnd());
		std::cout <<std::endl;
	}

	return 0;
}
