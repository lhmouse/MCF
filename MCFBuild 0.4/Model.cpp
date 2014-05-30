// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Model.hpp"
#include "../MCF/Serialization/Serdes.hpp"
#include <cstddef>
using namespace MCFBuild;

SERDES_TABLE_BEGIN(Sha256)
	SERDES_MEMBER(abyChecksum)
SERDES_TABLE_END
