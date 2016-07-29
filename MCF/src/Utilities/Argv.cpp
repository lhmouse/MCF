// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "Argv.hpp"
#include "../Core/Exception.hpp"
#include <MCFCRT/env/mcfwin.h>

namespace MCF {

Argv::Argv(const wchar_t *pwszCommandLine){
	if(!pwszCommandLine){
		pwszCommandLine = ::GetCommandLineW();
	}
	if(!x_pArgv.Reset(::_MCFCRT_AllocArgv(&x_uArgc, pwszCommandLine))){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"_MCFCRT_AllocArgv() 失败。"));
	}
}

}
