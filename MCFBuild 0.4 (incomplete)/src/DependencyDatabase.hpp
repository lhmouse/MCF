// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_DEPENDENCY_DATABASE_HPP_
#define MCFBUILD_DEPENDENCY_DATABASE_HPP_

#include "Model.hpp"
#include "../MCF/Core/String.hpp"
#include <map>

namespace MCFBuild {

struct DependencyItem {
	Sha256 shaCommandLine;
	Sha256 shaSource;
	Sha256 shaPreProcessed;
	std::map<MCF::WideString, Sha256> mapDependencies;
};

class DependencyDatabase {
public:
	std::map<MCF::WideString, DependencyItem> m_mapFiles;

public:
	void LoadFromFile(const MCF::WideString &wsoPath);
	void SaveToFile(const MCF::WideString &wsoPath) const;
};

}

#endif
