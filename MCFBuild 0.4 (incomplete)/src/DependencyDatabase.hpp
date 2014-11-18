// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_DEPENDENCY_DATABASE_HPP_
#define MCFBUILD_DEPENDENCY_DATABASE_HPP_

#include "../MCF/Core/String.hpp"
#include "FileSystem.hpp"
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
	using FileMap = std::map<MCF::WideString, DependencyItem>;

private:
	FileMap xm_mapFiles;

public:
	const FileMap &GetFileMap() const noexcept {
		return xm_mapFiles;
	}
	FileMap &GetFileMap() noexcept {
		return xm_mapFiles;
	}

	void SaveToFile(const MCF::WideString &wsoPath) const;
	void LoadFromFile(const MCF::WideString &wsoPath);
};

}

#endif
