// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_MODEL_HPP_
#define MCFBUILD_MODEL_HPP_

#include "../MCF/Utilities/Noncopyable.hpp"
#include "../MCF/Containers/VVector.hpp"
#include "../MCF/Containers/MultiIndexMap.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/File.hpp"
#include <cstddef>

namespace MCFBuild {

class Model : Noncopyable {
public:
	enum OPERATION {
		OP_SHOW_HELP,
		OP_BUILD,
		OP_CLEAN,
		OP_REBUILD
	};

public:
	static Model &GetInstance() noexcept;

private:
	const MCF::VVector<MCF::WideString> xm_vecArgV;
	bool xm_bRawOutput;

	bool xm_bVerbose;

	MCF::WideString xm_wcsProject;
	MCF::WideString xm_wcsConfig;
//	MCF::MultiIndexMap<MCF::WideString, MCF::WideString> xm_mapMacros;

	MCF::WideString xm_wcsWorkingDir;
	MCF::WideString xm_wcsSrcRoot;
	MCF::WideString xm_wcsIntermediateRoot;
	MCF::WideString xm_wcsDstRoot;

	OPERATION xm_eOperation;
	std::size_t xm_uProcessCount;

protected:
	Model();

public:
	void InitParams();

public:
	auto DoesUseRawOutput() const noexcept {
		return xm_bRawOutput;
	}

	auto IsVerbose() const noexcept {
		return xm_bVerbose;
	}

	const auto &GetProject() const noexcept {
		return xm_wcsProject;
	}
	const auto &GetConfig() const noexcept {
		return xm_wcsConfig;
	}
/*	const auto &GetMacros() const noexcept {
		return xm_mapMacros;
	}*/

	const auto &GetWorkingDir() const noexcept {
		return xm_wcsWorkingDir;
	}
	const auto &GetSrcRoot() const noexcept {
		return xm_wcsSrcRoot;
	}
	const auto &GetIntermediateRoot() const noexcept {
		return xm_wcsIntermediateRoot;
	}
	const auto &GetDstRoot() const noexcept {
		return xm_wcsDstRoot;
	}

	auto GetOperation() const noexcept {
		return xm_eOperation;
	}
	auto GetProcessCount() const noexcept {
		return xm_uProcessCount;
	}
};

}

#endif
