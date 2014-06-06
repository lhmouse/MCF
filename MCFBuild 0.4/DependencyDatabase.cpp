// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "DependencyDatabase.hpp"
#include "FileSystem.hpp"
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
	m_mapFiles.clear();
	MCF::StreamBuffer sbufData;
	GetFileContents(sbufData, wcsPath, true);
	MCF::Deserialize(*this, sbufData);
}
void DependencyDatabase::SaveToFile(const MCF::WideString &wcsPath) const {
	MCF::StreamBuffer sbufData;
	MCF::Serialize(sbufData, *this);
	PutFileContents(wcsPath, sbufData);
}
