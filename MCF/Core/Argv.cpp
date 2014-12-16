// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Argv.hpp"
#include "Exception.hpp"
using namespace MCF;

void ArgvResult::ArgItemDeleter::operator()(const ::MCF_ArgItem *pArgItem) const noexcept {
	::MCF_CRT_FreeArgv(pArgItem);
}

namespace MCF {

ArgvResult GetArgv(const wchar_t *pwszCommandLine){
	const char *pszFunction;
	ArgvResult vRet;
	if(pwszCommandLine){
		pszFunction = "MCF_CRT_AllocArgv";
		vRet.pArgv.reset(::MCF_CRT_AllocArgv(&vRet.uArgc, pwszCommandLine));
	} else {
		pszFunction = "MCF_CRT_AllocArgvFromCommandLine";
		vRet.pArgv.reset(::MCF_CRT_AllocArgvFromCommandLine(&vRet.uArgc));
	}
	if(!vRet.pArgv){
		DEBUG_THROW(SystemError, pszFunction);
	}
	return std::move(vRet);
}

}
