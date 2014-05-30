// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_MODEL_HPP_
#define MCFBUILD_MODEL_HPP_

#include "../MCF/Core/Utilities.hpp"
#include <cstddef>

namespace MCFBuild {

constexpr std::size_t MAX_FILE_SIZE = 64 * 0x400 * 0x400; // 64 MiB

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
	bool operator>(const Sha256 &rhs) const noexcept {
		return rhs < *this;
	}
	bool operator<=(const Sha256 &rhs) const noexcept {
		return !(rhs < *this);
	}
	bool operator>=(const Sha256 &rhs) const noexcept {
		return !(*this < rhs);
	}
};

}


#endif
