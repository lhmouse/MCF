#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/File.hpp>
#include <MCF/StreamFilters/LzmaFilters.hpp>
#include <MCF/Hash/Crc32.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
/*	try {
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

	VList<int> l1, l2;

	for(int i = 0; i < 10; ++i){
		l1.Push(100 + i);
	}
	for(int i = 0; i < 10; ++i){
		l2.Push(200 + i);
	}

	auto p1 = l1.GetBegin();
	for(int i = 0; i < 5; ++i){
		p1 = p1->GetNext();
	}
	auto p2 = l2.GetBegin();
	for(int i = 0; i < 5; ++i){
		p2 = p2->GetNext();
	}

	l1.Splice(p1, l2, l2.GetBegin(), p2);

	for(auto p = l1.GetBegin(); p; p = p->GetNext()){
		std::printf("%d\n", p->GetElement());
	}
	std::puts("----------");
	for(auto p = l2.GetBegin(); p; p = p->GetNext()){
		std::printf("%d\n", p->GetElement());
	}

	return 0;
}
