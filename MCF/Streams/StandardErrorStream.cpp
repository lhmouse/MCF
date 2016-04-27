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
						MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"无法获取标准错误流的句柄。"));
					}
					return hPipe;
				}())
		{
		}
		~Pipe(){
		}

		Pipe(const Pipe &) = delete;

	private:
		void X_UnbufferedWrite(const void *pData, std::size_t uSize) const {
			bool bStandardOutputStreamsFlushed = false;

			const auto pbyData = static_cast<const unsigned char *>(pData);
			std::size_t uBytesTotal = 0;
			for(;;){
				const auto dwBytesToWrite = static_cast<DWORD>(Min(uSize - uBytesTotal, UINT32_MAX));
				if(dwBytesToWrite == 0){
					break;
				}

				if(!bStandardOutputStreamsFlushed){
					StandardOutputStream().Flush(false);
					bStandardOutputStreamsFlushed = true;
				}

				DWORD dwBytesWritten;
				if(!::WriteFile(x_hPipe, pbyData + uBytesTotal, dwBytesToWrite, &dwBytesWritten, nullptr)){
					MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardErrorStream: WriteFile() 失败。"));
				}
				if(dwBytesWritten == 0){
					MCF_THROW(Exception, ERROR_BROKEN_PIPE, Rcntws::View(L"StandardErrorStream: 未能成功写入所有数据。"));
				}
				uBytesTotal += dwBytesWritten;
			}
		}

	public:
		bool IsNull() const noexcept {
			return !x_hPipe;
		}

		void Write(const void *pData, std::size_t uSize){
			return X_UnbufferedWrite(pData, uSize);
		}

		void Flush(bool bHard){
			if(bHard){
				if(!::FlushFileBuffers(x_hPipe)){
					const auto dwLastError = ::GetLastError();
					if((dwLastError != ERROR_INVALID_FUNCTION) && (dwLastError != ERROR_INVALID_HANDLE)){
						MCF_THROW(Exception, dwLastError, Rcntws::View(L"StandardErrorStream: FlushFileBuffers() 失败。"));
					}
				}
			}
		}
	};

	static_assert(std::is_trivially_destructible<RecursiveMutex>::value, "Please fix this!");

	__attribute__((__init_priority__(101)))
	RecursiveMutex g_vMutex;
	__attribute__((__init_priority__(103)))
	Pipe           g_vPipe;
}

StandardErrorStream::~StandardErrorStream(){
}

void StandardErrorStream::Put(unsigned char byData){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	g_vPipe.Write(&byData, 1);
}

void StandardErrorStream::Put(const void *pData, std::size_t uSize){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	g_vPipe.Write(pData, uSize);
}

void StandardErrorStream::Flush(bool bHard){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	g_vPipe.Flush(bHard);
}

}
