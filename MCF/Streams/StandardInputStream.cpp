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
#include "../Containers/StaticVector.hpp"
#include "StandardOutputStream.hpp"

namespace MCF {

namespace {
	class Pipe {
	private:
		const HANDLE x_hPipe;

		bool x_bEchoing = true;
		StreamBuffer x_vBuffer;
		StaticVector<unsigned char, 4096> x_vecBackBuffer;

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
		{
		}
		~Pipe(){
		}

		Pipe(const Pipe &) = delete;

	private:
		void X_PopulateBuffer(){
			bool bStandardOutputStreamsFlushed = false;

			for(;;){
				if(!x_vecBackBuffer.IsEmpty()){
					x_vBuffer.Put(x_vecBackBuffer.GetData(), x_vecBackBuffer.GetSize());
					x_vecBackBuffer.Clear();
				}
				if(!x_vBuffer.IsEmpty()){
					break;
				}

				if(!bStandardOutputStreamsFlushed){
					StandardOutputStream().Flush(false);
					bStandardOutputStreamsFlushed = true;
				}

				std::size_t uBytesRead;
				x_vecBackBuffer.Resize(4096);
				try {
					uBytesRead = X_UnbufferedRead(x_vecBackBuffer.GetData(), x_vecBackBuffer.GetSize());
					x_vecBackBuffer.Pop(x_vecBackBuffer.GetSize() - uBytesRead);
				} catch(...){
					x_vecBackBuffer.Clear();
					throw;
				}
				if(uBytesRead == 0){
					break;
				}
			}
		}

		std::size_t X_UnbufferedRead(void *pData, std::size_t uSize){
			const auto dwBytesToRead = static_cast<DWORD>(Min(uSize, UINT32_MAX));
			DWORD dwBytesRead;
			if(!::ReadFile(x_hPipe, pData, dwBytesToRead, &dwBytesRead, nullptr)){
				MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardInputStream: ReadFile() 失败。"));
			}
			return dwBytesRead;
		}

	public:
		bool IsNull() const noexcept {
			return !x_hPipe;
		}

		int Peek(){
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

		std::size_t Peek(void *pData, std::size_t uSize){
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

		bool IsEchoing() const noexcept {
			return x_bEchoing;
		}
		void SetEchoing(bool bEchoing){
			DWORD dwConsoleMode;
			if(::GetConsoleMode(x_hPipe, &dwConsoleMode)){
				constexpr DWORD kEchoingMask = ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT;
				if(bEchoing){
					dwConsoleMode |=  kEchoingMask;
				} else {
					dwConsoleMode &= ~kEchoingMask;
				}
				if(!::SetConsoleMode(x_hPipe, dwConsoleMode)){
					MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"StandardInputStream: SetConsoleMode() 失败。"));
				}
			}
			x_bEchoing = bEchoing;
		}
	};

	static_assert(std::is_trivially_destructible<RecursiveMutex>::value, "Please fix this!");

	RecursiveMutex g_vMutex __attribute__((__init_priority__(102)));
	Pipe           g_vPipe  __attribute__((__init_priority__(102)));
}

StandardInputStream::~StandardInputStream(){
}

int StandardInputStream::Peek(){
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

std::size_t StandardInputStream::Peek(void *pData, std::size_t uSize){
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

bool StandardInputStream::IsEchoing() const noexcept {
	if(g_vPipe.IsNull()){
		return false;
	}
	const auto vLock = g_vMutex.GetLock();

	return g_vPipe.IsEchoing();
}
void StandardInputStream::SetEchoing(bool bEchoing){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	g_vPipe.SetEchoing(bEchoing);
}

}
