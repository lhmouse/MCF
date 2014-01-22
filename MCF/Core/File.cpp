// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "File.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
#include "Event.hpp"
#include "Thread.hpp"
using namespace MCF;

// 嵌套类定义。
class File::xDelegate {
private:
	struct xFileCloser {
		constexpr HANDLE operator()() const {
			return INVALID_HANDLE_VALUE;
		}
		void operator()(HANDLE hFile) const {
			::CloseHandle(hFile);
		}
	};

	struct xAsyncControlBlock {
		Event m_evnIdle;
		Thread m_thrdWorker;
	};
private:
	std::unique_ptr<xAsyncControlBlock> xm_pAsyncControlBlock;
	UniqueHandle<HANDLE, xFileCloser> xm_hFile;
	LARGE_INTEGER xm_liFileSize;
	unsigned long xm_ulErrorCode;
public:
	xDelegate(){
	}
public:
};

// 构造函数和析构函数。
File::File()
	: xm_pDelegate()
{
}
File::~File(){
}

// 其他非静态成员函数。
