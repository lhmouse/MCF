#include <MCF/StdMCF.hpp>
#include <MCF/Core/LastError.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Utilities/Bail.hpp>
#include <cstdio>
#include <winternl.h>
#include <ntdef.h>
#include <ntstatus.h>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	const auto code = ::RtlNtStatusToDosError(STATUS_RECEIVE_PARTIAL);
	std::printf("code = %lx\n", code);
	Bail(WideString(GetWin32ErrorDescription(code)).GetStr());
	return 0;
}
