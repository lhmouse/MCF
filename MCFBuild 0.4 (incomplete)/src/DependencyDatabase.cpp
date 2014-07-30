// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "DependencyDatabase.hpp"
#include "FileSystem.hpp"
#include "../MCF/Containers/VVector.hpp"
#include "../MCF/Core/Exception.hpp"
#include "../MCF/Serialization/Serdes.hpp"
#include <cstddef>
using namespace MCFBuild;

void DependencyDatabase::LoadFromFile(const MCF::WideString &wcsPath){
//	m_mapFiles.clear();
//	MCF::StreamBuffer sbufData;
//	GetFileContents(sbufData, wcsPath, true);
//	MCF::Deserialize(*this, sbufData);
}
void DependencyDatabase::SaveToFile(const MCF::WideString &wcsPath) const {
//	MCF::StreamBuffer sbufData;
//	MCF::Serialize(sbufData, *this);
//	PutFileContents(wcsPath, sbufData);
}
