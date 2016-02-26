// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardInputStream.hpp"
#include "../Utilities/Bail.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Thread/RecursiveMutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/StreamBuffer.hpp"
#include "StandardOutputStream.hpp"

namespace MCF {

namespace {
	class Pipe {
	private:
		const HANDLE x_hPipe;

		mutable StreamBuffer x_vBuffer;
		mutable unsigned char x_abyBackBuffer[4096];
		mutable std::size_t x_uBackBufferSize;

	public:
		Pipe()
			: x_hPipe(
				[]{
					const auto hPipe = ::GetStdHandle(STD_INPUT_HANDLE);
					if(hPipe == INVALID_HANDLE_VALUE){
						const auto dwLastError = ::GetLastError();
						BailF(L"无法获取标准输入流的句柄。\n\n错误码：%lu", (unsigned long)dwLastError);
					}
					return hPipe;
				}())
			, x_uBackBufferSize(0)
		{
		}
		~Pipe(){
		}

		Pipe(const Pipe &) = delete;

	private:
		void X_PopulateBuffer() const {
			bool bStandardOutputStreamsFlushed = false;

			for(;;){
				if(x_uBackBufferSize != 0){
					x_vBuffer.Put(x_abyBackBuffer, x_uBackBufferSize);
					x_uBackBufferSize = 0;
				}
				if(!x_vBuffer.IsEmpty()){
					break;
				}

				if(!bStandardOutputStreamsFlushed){
					StandardOutputStream::FlushAll(false);
					bStandardOutputStreamsFlushed = true;
				}

				DWORD dwBytesRead;
				if(!::ReadFile(x_hPipe, x_abyBackBuffer, sizeof(x_abyBackBuffer), &dwBytesRead, nullptr)){
					const auto dwLastError = ::GetLastError();
					DEBUG_THROW(SystemException, dwLastError, "ReadFile"_rcs);
				}
				if(dwBytesRead == 0){
					break;
				}
				x_uBackBufferSize = dwBytesRead;
			}
		}

	public:
		bool IsNull() const noexcept {
			return !x_hPipe;
		}

		int Peek() const {
			X_PopulateBuffer();
			return x_vBuffer.Peek();
		}
		int Get(){
			X_PopulateBuffer();
			return x_vBuffer.Get();
		}
		bool Discard(){
			X_PopulateBuffer();
			return x_vBuffer.Discard();
		}

		std::size_t Peek(void *pData, std::size_t uSize) const {
			X_PopulateBuffer();
			return x_vBuffer.Peek(pData, uSize);
		}
		std::size_t Get(void *pData, std::size_t uSize){
			X_PopulateBuffer();
			return x_vBuffer.Get(pData, uSize);
		}
		std::size_t Discard(std::size_t uSize){
			X_PopulateBuffer();
			return x_vBuffer.Discard(uSize);
		}
	};

	static_assert(std::is_trivially_destructible<RecursiveMutex>::value, "Please fix this!");

	RecursiveMutex g_vMutex __attribute__((__init_priority__(101)));
	Pipe           g_vPipe  __attribute__((__init_priority__(101)));
}

StandardInputStream::~StandardInputStream(){
}

int StandardInputStream::Peek() const {
	if(g_vPipe.IsNull()){
		return -1;
	}
	const auto vLock = g_vMutex.GetLock();

	return g_vPipe.Peek();
}
int StandardInputStream::Get(){
	if(g_vPipe.IsNull()){
		return -1;
	}
	const auto vLock = g_vMutex.GetLock();

	return g_vPipe.Get();
}
bool StandardInputStream::Discard(){
	if(g_vPipe.IsNull()){
		return false;
	}
	const auto vLock = g_vMutex.GetLock();

	return g_vPipe.Discard();
}

std::size_t StandardInputStream::Peek(void *pData, std::size_t uSize) const {
	if(g_vPipe.IsNull()){
		return 0;
	}
	const auto vLock = g_vMutex.GetLock();

	return g_vPipe.Peek(pData, uSize);
}
std::size_t StandardInputStream::Get(void *pData, std::size_t uSize){
	if(g_vPipe.IsNull()){
		return 0;
	}
	const auto vLock = g_vMutex.GetLock();

	return g_vPipe.Get(pData, uSize);
}
std::size_t StandardInputStream::Discard(std::size_t uSize){
	if(g_vPipe.IsNull()){
		return 0;
	}
	const auto vLock = g_vMutex.GetLock();

	return g_vPipe.Discard(uSize);
}

}
