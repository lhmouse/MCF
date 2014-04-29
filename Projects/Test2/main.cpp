#include <MCF/StdMCF.hpp>
#include <MCF/Core/File.hpp>
#include <MCF/Core/VVector.hpp>
#include <MCF/Hash/Crc32.hpp>
using namespace MCF;

unsigned int MCFMain(){
	auto pFile = File::Open(L"F:\\Downloads\\cpumemory.pdf", true, false, false);
	VVector<unsigned char> vecData(pFile->GetSize());
	pFile->Read(vecData.GetData(), vecData.GetSize(), 0);

	Crc32 vCrc32;
	vCrc32.Update(vecData.GetData(), vecData.GetSize());
	std::uint32_t u32Crc = vCrc32.Finalize();
	std::printf("%08lX\n", (unsigned long)u32Crc);

	return 0;
}
