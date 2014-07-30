// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Serdes.hpp"
#include "../Core/Exception.hpp"
using namespace MCF;

namespace MCF {

[[noreturn]]
void ThrowEndOfStream(){
	MCF_THROW(ERROR_HANDLE_EOF, L"遇到文件尾。"_wso);
}

[[noreturn]]
void ThrowInvalidData(){
	MCF_THROW(ERROR_INVALID_DATA, L"数据损坏。"_wso);
}

}
