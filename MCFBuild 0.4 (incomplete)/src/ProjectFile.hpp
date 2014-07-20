// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_PROJECT_FILE_HPP_
#define MCFBUILD_PROJECT_FILE_HPP_

#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Language/Notation.hpp"
#include <cstddef>

namespace MCFBuild {

class ProjectFile {
private:
	MCF::Notation xm_vData;

public:
	explicit ProjectFile(const MCF::WideString &wcsFullPath);
};

}

#endif
