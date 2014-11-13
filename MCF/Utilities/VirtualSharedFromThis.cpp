// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "VirtualSharedFromThis.hpp"
using namespace MCF;

// 虚函数定义在这里，我们在共享库中只保存一份 RTTI 信息。
VirtualSharedFromThis::~VirtualSharedFromThis(){
}
