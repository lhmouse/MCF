// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "AbstractExeRootClass.hpp"
#include <vector>
using namespace MCF;

// 静态成员变量定义。
std::atomic<AbstractExeRootClass *> AbstractExeRootClass::xs_pLastLatentExeRoot(nullptr);

// 构造函数和析构函数。
AbstractExeRootClass::AbstractExeRootClass(){
	xm_pNext = xs_pLastLatentExeRoot.exchange(this);

	xm_pvecArgVector = nullptr;
}
AbstractExeRootClass::~AbstractExeRootClass(){
}

// 其他非静态成员函数。
int AbstractExeRootClass::xThreadProc(){
	ASSERT(xm_pvecArgVector != nullptr);

	return xMCFMain(xm_pvecArgVector->size() - 1, xm_pvecArgVector->data(), xm_nCmdShow);
}

HINSTANCE AbstractExeRootClass::GetHInstance() const {
	return xm_hInstance;
}
