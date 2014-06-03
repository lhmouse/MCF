// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "DependencyDatabase.hpp"
#include "../MCF/Core/File.hpp"
#include "../MCF/Core/VVector.hpp"
#include "../MCF/Core/Exception.hpp"
#include "../MCF/Serialization/Serdes.hpp"
#include <cstddef>
using namespace MCFBuild;

SERDES_TABLE_BEGIN(DependencyDatabase::FileItem)
	SERDES_MEMBER(shaCommandLine)
	SERDES_MEMBER(shaSource)
	SERDES_MEMBER(shaPreProcessed)
	SERDES_MEMBER(mapDependencies)
SERDES_TABLE_END

SERDES_TABLE_BEGIN(DependencyDatabase)
	SERDES_MEMBER(m_mapFiles)
SERDES_TABLE_END

void DependencyDatabase::LoadFromFile(const MCF::WideString &wcsPath){
	const auto pFile = MCF::File::Open(wcsPath, true, false, false);
	const unsigned long long ullFileSize = pFile->GetSize();
	if(ullFileSize >= MAX_FILE_SIZE){
		MCF_THROW(ERROR_INVALID_DATA, L"FILE_TOO_LARGE|"_ws + wcsPath);
	}

	MCF::StreamBuffer sbufData;
	while(sbufData.GetSize() < ullFileSize){
		unsigned char abyTemp[0x10000];
		const auto uBytesRead = pFile->Read(abyTemp, sizeof(abyTemp), sbufData.GetSize());
		sbufData.Insert(abyTemp, uBytesRead);
	}
	MCF::Deserialize(*this, sbufData);
}
void DependencyDatabase::SaveToFile(const MCF::WideString &wcsPath) const {
	MCF::StreamBuffer sbufData;
	MCF::Serialize(sbufData, *this);

	const auto pFile = MCF::File::Open(wcsPath, false, true, true);
	pFile->Clear();
	unsigned long long ullBytesWritten = 0;
	sbufData.Traverse(
		[&](auto pbyData, auto uSize){
			pFile->Write(ullBytesWritten, pbyData, uSize);
			ullBytesWritten += uSize;
		}
	);
}
