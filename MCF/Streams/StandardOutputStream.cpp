// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardOutputStream.hpp"
#include "../Core/Exception.hpp"
#include "../Thread/RecursiveMutex.hpp"

namespace MCF {

namespace {
	static_assert(std::is_trivially_destructible<RecursiveMutex>::value, "Please fix this!");

	RecursiveMutex         g_mtxListMutex;
	StandardOutputStream * g_pFirst;
	StandardOutputStream * g_pLast;

	HANDLE GetStdOutputHandle(){
		const auto hPipe = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if(hPipe == INVALID_HANDLE_VALUE){
			const auto dwLastError = ::GetLastError();
			DEBUG_THROW(SystemException, dwLastError, "GetStdHandle"_rcs);
		}
		return hPipe;
	}

	void FlushBuffer(HANDLE hPipe, StreamBuffer &vBuffer, std::size_t uThreshold){
		for(;;){
			if(vBuffer.GetSize() < uThreshold){
				break;
			}

			unsigned char abyTemp[4096];
			const auto uBytesToWrite = vBuffer.Peek(abyTemp, sizeof(abyTemp));
			if(uBytesToWrite == 0){
				break;
			}
			DWORD dwBytesWritten;
			if(!::WriteFile(hPipe, abyTemp, static_cast<DWORD>(uBytesToWrite), &dwBytesWritten, nullptr)){
				const auto dwLastError = ::GetLastError();
				DEBUG_THROW(SystemException, dwLastError, "WriteFile"_rcs);
			}
			vBuffer.Discard(dwBytesWritten);
		}
	}
}

void StandardOutputStream::FlushAll(bool bHard){
	const auto vLock = g_mtxListMutex.GetLock();

	for(auto pStream = g_pFirst; pStream; pStream = pStream->x_pNext){
		pStream->StandardOutputStream::Flush(bHard);
	}
}

StandardOutputStream::StandardOutputStream()
	: x_hPipe(GetStdOutputHandle())
{
	{
		const auto vLock = g_mtxListMutex.GetLock();

		x_pPrev = g_pLast;
		x_pNext = nullptr;

		if(x_pPrev){
			x_pPrev->x_pNext = this;
		} else {
			g_pFirst = this;
		}
//		if(x_pNext){
//			x_pNext->x_pPrev = this;
//		} else {
			g_pLast = this;
//		}
	}
}
StandardOutputStream::~StandardOutputStream(){
	{
		const auto vLock = g_mtxListMutex.GetLock();

		if(x_pPrev){
			x_pPrev->x_pNext = x_pNext;
		} else {
			g_pFirst = x_pNext;
		}
		if(x_pNext){
			x_pNext->x_pPrev = x_pPrev;
		} else {
			g_pLast = x_pPrev;
		}
	}

	if(!x_hPipe){
		return;
	}

	try {
		FlushBuffer(x_hPipe, x_vBuffer, 0);
	} catch(...){
	}
}

void StandardOutputStream::Put(unsigned char byData){
	if(!x_hPipe){
		return;
	}

	x_vBuffer.Put(byData);
	FlushBuffer(x_hPipe, x_vBuffer, 4096);
}

void StandardOutputStream::Put(const void *pData, std::size_t uSize){
	if(!x_hPipe){
		return;
	}

	x_vBuffer.Put(pData, uSize);
	FlushBuffer(x_hPipe, x_vBuffer, 4096);
}

void StandardOutputStream::Flush(bool bHard) const {
	if(!x_hPipe){
		return;
	}

	FlushBuffer(x_hPipe, x_vBuffer, 0);

	if(bHard){
		if(!::FlushFileBuffers(x_hPipe)){
			const auto dwLastError = ::GetLastError();
			if((dwLastError != ERROR_INVALID_FUNCTION) && (dwLastError != ERROR_INVALID_HANDLE)){
				DEBUG_THROW(SystemException, dwLastError, "FlushFileBuffers"_rcs);
			}
		}
	}
}

}
