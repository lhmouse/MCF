// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardInputStream.hpp"
#include "StandardOutputStream.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

namespace {
	HANDLE GetStdInputHandle(){
		const auto hPipe = ::GetStdHandle(STD_INPUT_HANDLE);
		if(hPipe == INVALID_HANDLE_VALUE){
			const auto dwLastError = ::GetLastError();
			DEBUG_THROW(SystemException, dwLastError, "GetStdHandle"_rcs);
		}
		return hPipe;
	}

	void PopulateBuffer(StreamBuffer &vBuffer, Vector<unsigned char> &vecBackBuffer, HANDLE hPipe){
		bool bStandardOutputStreamsFlushed = false;

		for(;;){
			if(!vecBackBuffer.IsEmpty()){
				vBuffer.Put(vecBackBuffer.GetData(), vecBackBuffer.GetSize());
				vecBackBuffer.Clear();
			}
			if(!vBuffer.IsEmpty()){
				break;
			}

			if(!bStandardOutputStreamsFlushed){
				StandardOutputStream::FlushAll(false);
				bStandardOutputStreamsFlushed = true;
			}

			vecBackBuffer.Resize(4096);
			try {
				DWORD dwBytesRead;
				if(!::ReadFile(hPipe, vecBackBuffer.GetData(), vecBackBuffer.GetSize(), &dwBytesRead, nullptr)){
					const auto dwLastError = ::GetLastError();
					DEBUG_THROW(SystemException, dwLastError, "ReadFile"_rcs);
				}
				vecBackBuffer.Pop(vecBackBuffer.GetSize() - dwBytesRead);
			} catch(...){
				vecBackBuffer.Clear();
				throw;
			}
			if(vecBackBuffer.IsEmpty()){
				break;
			}
		}
	}
}

StandardInputStream::StandardInputStream()
	: x_hPipe(GetStdInputHandle())
{
}
StandardInputStream::~StandardInputStream(){
}

int StandardInputStream::Peek() const {
	if(!x_hPipe){
		return -1;
	}

	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_hPipe);
	return x_vBuffer.Peek();
}
int StandardInputStream::Get(){
	if(!x_hPipe){
		return -1;
	}

	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_hPipe);
	return x_vBuffer.Get();
}
bool StandardInputStream::Discard(){
	if(!x_hPipe){
		return false;
	}

	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_hPipe);
	return x_vBuffer.Discard();
}

std::size_t StandardInputStream::Peek(void *pData, std::size_t uSize) const {
	if(!x_hPipe){
		return 0;
	}

	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_hPipe);
	return x_vBuffer.Get(pData, uSize);
}
std::size_t StandardInputStream::Get(void *pData, std::size_t uSize){
	if(!x_hPipe){
		return 0;
	}

	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_hPipe);
	return x_vBuffer.Peek(pData, uSize);
}
std::size_t StandardInputStream::Discard(std::size_t uSize){
	if(!x_hPipe){
		return 0;
	}

	PopulateBuffer(x_vBuffer, x_vecBackBuffer, x_hPipe);
	return x_vBuffer.Discard(uSize);
}

}
