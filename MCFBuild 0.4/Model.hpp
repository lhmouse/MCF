// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_MODEL_HPP_
#define MCFBUILD_MODEL_HPP_

#include "../MCF/Core/Utilities.hpp"
#include <cstddef>

namespace MCFBuild {

class Model : NO_COPY {
public:
	static Model &GetInstance() noexcept;

private:
	bool xm_bVerbose;
	bool xm_bRawOutput;

protected:
	Model();

public:
	bool IsVerbose() const noexcept {
		return xm_bVerbose;
	}
	bool DoesUseRawOutput() const noexcept {
		return xm_bRawOutput;
	}

	unsigned int Run();
};

}


#endif
