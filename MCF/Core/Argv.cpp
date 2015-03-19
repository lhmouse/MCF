// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Argv.hpp"
#include "Exception.hpp"

namespace MCF {

Argv::Argv(const wchar_t *pwszCommandLine){
	if(!pwszCommandLine){
		pwszCommandLine = ::GetCommandLineW();
	}
	if(!x_pArgv.Reset(::MCF_CRT_AllocArgv(&x_uArgc, pwszCommandLine))){
		DEBUG_THROW(SystemError, "MCF_CRT_AllocArgv");
	}
}

}
