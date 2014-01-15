// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "Lib.hpp"
using namespace MCF;

namespace MCF {
	namespace {
		CriticalSection xGlobalCriticalSection;
	}

	extern void EnterGlobalCriticalSection(){
		xGlobalCriticalSection.Enter();
	}
	extern bool TryGlobalCriticalSection(){
		return xGlobalCriticalSection.Try();
	}
	extern void LeaveGlobalCriticalSection(){
		xGlobalCriticalSection.Leave();
	}
}
