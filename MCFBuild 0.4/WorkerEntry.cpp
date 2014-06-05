// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "FileSystem.hpp"
#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/VVector.hpp"
#include "../MCF/Core/Exception.hpp"
#include <map>
using namespace MCFBuild;

namespace MCFBuild {

unsigned int WorkerEntry(const MCF::VVector<MCF::WideStringObserver> &vecArgs){
	bool bShowHelp = true; //false;

	MCF::WideString wcsProjFile;
	MCF::WideString wcsConfig;
	std::map<MCF::WideString, std::pair<MCF::WideString, bool>> mapMacros;

	MCF::WideString wcsRootDir;
	MCF::WideString wcsSrcDir;
	MCF::WideString wcsDstDir;

	enum {
		OP_BUILD,
		OP_REBUILD,
		OP_CLEANUP
	} eOperation = OP_BUILD;

	unsigned long ulProcessCount = 0;
	bool bVerbose = false;

	//

	if(bShowHelp){
		PrintLn(FormatString(L"MCFBUILD_USAGE"_wso));
		return 0;
	}

	return 0;
}

}
