#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Socket/UdpServer.hpp>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	printf("%f\n", ::fabsf(2.0f));
	printf("%f\n", ::fabsf(-2.0f));
	printf("%f\n", ::fabs(3.0));
	printf("%f\n", ::fabs(-3.0));
	printf("%Lf\n", ::fabsl(4.0l));
	printf("%Lf\n", ::fabsl(-4.0l));
	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->m_ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->m_pszFunction);
		printf("  line = %lu\n", p->m_ulLine);
		printf("  msg  = %s\n", AnsiString(WideString(p->m_wcsMessage)).GetCStr());
	}
	return 0;
}
