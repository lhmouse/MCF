// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_DEPENDENCY_DATABASE_HPP_
#define MCFBUILD_DEPENDENCY_DATABASE_HPP_

#include "../MCF/Core/String.hpp"
#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Serialization/SerdesDecl.hpp"
#include <map>

namespace MCFBuild {

class DependencyDatabase {
	SERDES_FRIEND_DECL(DependencyDatabase);

public:
	struct Sha256 {
		unsigned char abyChecksum[32];

		bool operator==(const Sha256 &rhs) const noexcept {
			return MCF::BComp(abyChecksum, rhs.abyChecksum) == 0;
		}
		bool operator!=(const Sha256 &rhs) const noexcept {
			return !(*this == rhs);
		}
		bool operator<(const Sha256 &rhs) const noexcept {
			return MCF::BComp(abyChecksum, rhs.abyChecksum) < 0;
		}
	};

	struct FileItem {
		Sha256 shaCommandLine;
		Sha256 shaFile;
		std::map<MCF::WideString, Sha256> mapDependencies;
	};

public:
	Sha256 m_shaSelf;
	std::map<MCF::WideString, FileItem> m_mapFiles;

public:
	void LoadFromFile(const MCF::WideString &wsoPath);
	void SaveToFile(const MCF::WideString &wsoPath) const;
};

}

#endif
