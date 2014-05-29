// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_SerdesTraits.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

namespace Impl {
	[[noreturn]] void ThrowOnEof(){
		MCF_THROW(ERROR_HANDLE_EOF, L"遇到意外的文件尾。");
	}
}

}
