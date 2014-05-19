// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Serializer.hpp"
#include "../Core/Exception.hpp"
using namespace MCF;

// 其他非静态成员函数。
[[noreturn]] void Impl::SerializerBase::xBailOnEndOfStream() const {
	MCF_THROW(ERROR_HANDLE_EOF, L"遇到文件尾。");
}
