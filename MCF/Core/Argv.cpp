// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Argv.hpp"
#include "Exception.hpp"
using namespace MCF;

void ArgvResult::ArgItemDeleter::operator()(::MCF_ArgItem *pArgItem) const noexcept {
	::MCF_CRT_FreeArgv(pArgItem);
}

namespace MCF {

ArgvResult GetArgv(const wchar_t *pwszCommandLine){
	ArgvResult vRet;
	if(pwszCommandLine){
		if(!vRet.pArgv.Reset(::MCF_CRT_AllocArgv(&vRet.uArgc, pwszCommandLine))){
			DEBUG_THROW(SystemError, "MCF_CRT_AllocArgv");
		}
	} else {
		if(!vRet.pArgv.Reset(::MCF_CRT_AllocArgvFromCommandLine(&vRet.uArgc))){
			DEBUG_THROW(SystemError, "MCF_CRT_AllocArgvFromCommandLine");
		}
	}
	return vRet;
}

}
