// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

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
void Transaction::Clear(){
	xm_vecOperations.Clear();
}

void Transaction::CommitAll(){
	const auto ppBegin = xm_vecOperations.GetBegin();
	const auto ppEnd = xm_vecOperations.GetEnd();
	if(ppBegin == ppEnd){
		return;
	}

	auto ppHead = ppBegin;
	try {
		while(ppHead != ppEnd){
			(*ppHead)->Lock();
			++ppHead;
		}
	} catch(...){
		while(ppHead != ppBegin){
			--ppHead;
			(*ppHead)->Unlock();
		}
		throw;
	}
	ppHead = ppBegin;
	while(ppHead != ppEnd){
		(*ppHead)->Commit();
		++ppHead;
	}
	while(ppHead != ppBegin){
		--ppHead;
		(*ppHead)->Unlock();
	}
	xm_vecOperations.Clear();
}
