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

inline namespace {
	void operator>>=(const DependencyItem &vSource, MCF::StreamBuffer &sbufSink){
		vSource.shaCommandLine	>>= sbufSink;
		vSource.shaSource		>>= sbufSink;
		vSource.shaPreProcessed	>>= sbufSink;
		vSource.mapDependencies	>>= sbufSink;
	}
	void operator<<=(DependencyItem &vSink, MCF::StreamBuffer &sbufSource){
		vSink.shaCommandLine	<<= sbufSource;
		vSink.shaSource			<<= sbufSource;
		vSink.shaPreProcessed	<<= sbufSource;
		vSink.mapDependencies	<<= sbufSource;
	}
}

}

void DependencyDatabase::SaveToFile(const MCF::WideString &wcsPath) const {
	MCF::StreamBuffer sbufData;
	sbufData <<xm_mapFiles;
	PutFileContents(wcsPath, sbufData);
}
void DependencyDatabase::LoadFromFile(const MCF::WideString &wcsPath){
	xm_mapFiles.clear();
	MCF::StreamBuffer sbufData;
	GetFileContents(sbufData, wcsPath, true);
	sbufData >>xm_mapFiles;
}
