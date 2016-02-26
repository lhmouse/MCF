// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardErrorStream.hpp"
#include "../Utilities/Bail.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Core/Exception.hpp"
#include "../Thread/RecursiveMutex.hpp"
#include "StandardOutputStream.hpp"

namespace MCF {

namespace {
	class Pipe {
	private:
		const HANDLE x_hPipe;

	public:
		Pipe()
			: x_hPipe(
				[]{
					const auto hPipe = ::GetStdHandle(STD_ERROR_HANDLE);
					if(hPipe == INVALID_HANDLE_VALUE){
						const auto dwLastError = ::GetLastError();
						BailF(L"无法获取标准错误流的句柄。\n\n错误码：%lu", (unsigned long)dwLastError);
					}
					return hPipe;
				}())
		{
		}
		~Pipe(){
		}

		Pipe(const Pipe &) = delete;

	private:
		std::size_t X_UnbufferedWrite(const void *pData, std::size_t uSize){
			bool bStandardOutputStreamsFlushed = false;

			const auto pbyData = static_cast<const unsigned char *>(pData);
			std::size_t uBytesTotal = 0;
			for(;;){
				auto uBytesToWrite = Min(uSize - uBytesTotal, UINT32_MAX);
				if(uBytesToWrite == 0){
					break;
				}

				if(!bStandardOutputStreamsFlushed){
					StandardOutputStream::FlushAll(false);
					bStandardOutputStreamsFlushed = true;
				}

				DWORD dwBytesWritten;
				if(!::WriteFile(x_hPipe, pbyData + uBytesTotal, uBytesToWrite, &dwBytesWritten, nullptr)){
					const auto dwLastError = ::GetLastError();
					DEBUG_THROW(SystemException, dwLastError, "WriteFile"_rcs);
				}
				if(dwBytesWritten == 0){
					break;
				}
				uBytesTotal += dwBytesWritten;
			}
			return uBytesTotal;
		}

	public:
		bool IsNull() const noexcept {
			return !x_hPipe;
		}

		std::size_t Write(const void *pData, std::size_t uSize){
			return X_UnbufferedWrite(pData, uSize);
		}

		void Flush(bool bHard){
			if(bHard){
				if(!::FlushFileBuffers(x_hPipe)){
					const auto dwLastError = ::GetLastError();
					if((dwLastError != ERROR_INVALID_FUNCTION) && (dwLastError != ERROR_INVALID_HANDLE)){
						DEBUG_THROW(SystemException, dwLastError, "FlushFileBuffers"_rcs);
					}
				}
			}
		}
	};

	static_assert(std::is_trivially_destructible<RecursiveMutex>::value, "Please fix this!");

	RecursiveMutex g_vMutex __attribute__((__init_priority__(101)));
	Pipe           g_vPipe  __attribute__((__init_priority__(101)));
}

StandardErrorStream::~StandardErrorStream(){
}

void StandardErrorStream::Put(unsigned char byData){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	const auto uBytesWritten = g_vPipe.Write(&byData, 1);
	if(uBytesWritten < 1){
		DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "StandardErrorStream: Partial contents written"_rcs);
	}
}

void StandardErrorStream::Put(const void *pData, std::size_t uSize){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	const auto uBytesWritten = g_vPipe.Write(pData, uSize);
	if(uBytesWritten < uSize){
		DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "StandardErrorStream: Partial contents written"_rcs);
	}
}

void StandardErrorStream::Flush(bool bHard) const {
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	g_vPipe.Flush(bHard);
}

}
