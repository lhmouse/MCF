// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Transaction.hpp"
using namespace MCF;

// 其他非静态成员函数。
bool Transaction::IsEmpty() const noexcept {
	return xm_vecOperations.IsEmpty();
}
void Transaction::AddOperation(std::unique_ptr<OperationBase> pOperation){
	xm_vecOperations.Push(std::move(pOperation));
}
void Transaction::Clear() noexcept {
	xm_vecOperations.Clear();
}

bool Transaction::Commit() const {
	auto ppCur = xm_vecOperations.GetBegin();
	try {
		while(ppCur != xm_vecOperations.GetEnd()){
			if(!(*ppCur)->xLock()){
				break;
			}
			++ppCur;
		}
	} catch(...){
		while(ppCur != xm_vecOperations.GetBegin()){
			--ppCur;
			(*ppCur)->xUnlock();
		}
		throw;
	}
	if(ppCur != xm_vecOperations.GetEnd()){
		while(ppCur != xm_vecOperations.GetBegin()){
			--ppCur;
			(*ppCur)->xUnlock();
		}
		return false;
	}
	ppCur = xm_vecOperations.GetBegin();
	while(ppCur != xm_vecOperations.GetEnd()){
		(*ppCur)->xCommit();
		++ppCur;
	}
	while(ppCur != xm_vecOperations.GetBegin()){
		--ppCur;
		(*ppCur)->xUnlock();
	}
	return true;
}
