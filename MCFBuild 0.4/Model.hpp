// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_MODEL_HPP_
#define MCFBUILD_MODEL_HPP_

#include "../MCF/Core/Utilities.hpp"
#include <array>
#include <cstddef>

namespace MCFBuild {

constexpr std::size_t MAX_FILE_SIZE = 64 * 0x400 * 0x400; // 64 MiB

typedef std::array<unsigned char, 32> Sha256;

}


#endif
