// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_UniqueWin32Handle.hpp"
using namespace MCF;

void Win32HandleCloser::operator()(void *hObject) const noexcept {
	::CloseHandle(hObject);
}
