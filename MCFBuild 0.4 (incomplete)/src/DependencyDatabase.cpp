// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "DependencyDatabase.hpp"
#include "FileSystem.hpp"
#include "../MCF/Core/Exception.hpp"
#include "../MCF/Serialization/Serdes.hpp"
#include "../MCF/Serialization/Serdes_array.hpp"
#include "../MCF/Serialization/Serdes_map.hpp"
#include <cstddef>
using namespace MCFBuild;

namespace MCFBuild {

namespace {
	void Serialize(MCF::StreamBuffer &sbufSink, const DependencyItem &vSource){
		MCF::Serialize(sbufSink, vSource.shaCommandLine);
		MCF::Serialize(sbufSink, vSource.shaSource);
		MCF::Serialize(sbufSink, vSource.shaPreProcessed);
		MCF::Serialize(sbufSink, vSource.mapDependencies);
	}
	void Deserialize(DependencyItem &vSink, MCF::StreamBuffer &sbufSource){
		MCF::Deserialize(vSink.shaCommandLine,	sbufSource);
		MCF::Deserialize(vSink.shaSource,		sbufSource);
		MCF::Deserialize(vSink.shaPreProcessed,	sbufSource);
		MCF::Deserialize(vSink.mapDependencies,	sbufSource);
	}
}

}

void DependencyDatabase::SaveToFile(const MCF::WideString &wcsPath) const {
	MCF::StreamBuffer sbufData;
//	MCF::Serialize(sbufData, xm_mapFiles);
	PutFileContents(wcsPath, sbufData);
}
void DependencyDatabase::LoadFromFile(const MCF::WideString &wcsPath){
	xm_mapFiles.clear();
	MCF::StreamBuffer sbufData;
	GetFileContents(sbufData, wcsPath, true);
//	MCF::Deserialize(xm_mapFiles, sbufData);
}
