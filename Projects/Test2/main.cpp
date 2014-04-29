#include <MCF/StdMCF.hpp>
#include <MCF/Core/File.hpp>
#include <MCF/Core/VVector.hpp>
#include <MCF/Hash/Sha1.hpp>
using namespace MCF;

unsigned int MCFMain(){
	auto pFile = File::Open(L"F:\\Downloads\\cpumemory.pdf", true, false, false);
	VVector<unsigned char> vecData(pFile->GetSize());
	pFile->Read(vecData.GetData(), vecData.GetSize(), 0);

	Sha1 vSha1;
	vSha1.Update(vecData.GetData(), vecData.GetSize());
	unsigned char abySha1[20];
	vSha1.Finalize(abySha1);
	for(auto by : abySha1){
		std::printf("%02hhX", by);
	}
	std::putchar('\n');

	return 0;
}
