// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Transaction.hpp"
using namespace MCF;

// 嵌套类。
TransactionItemBase::~TransactionItemBase(){
}

// 其他非静态成员函数。
bool Transaction::IsEmpty() const noexcept {
	return xm_vecItems.IsEmpty();
}
void Transaction::AddItem(UniquePtr<TransactionItemBase> &&pItem){
	xm_vecItems.Push(std::move(pItem));
}
void Transaction::Clear() noexcept {
	xm_vecItems.Clear();
}

bool Transaction::Commit() const {
	auto ppCur = xm_vecItems.GetBegin();
	try {
		while(ppCur != xm_vecItems.GetEnd()){
			if(!(*ppCur)->Lock()){
				break;
			}
			++ppCur;
		}
	} catch(...){
		while(ppCur != xm_vecItems.GetBegin()){
			--ppCur;
			(*ppCur)->Unlock();
		}
		throw;
	}
	if(ppCur != xm_vecItems.GetEnd()){
		while(ppCur != xm_vecItems.GetBegin()){
			--ppCur;
			(*ppCur)->Unlock();
		}
		return false;
	}
	ppCur = xm_vecItems.GetBegin();
	while(ppCur != xm_vecItems.GetEnd()){
		(*ppCur)->Commit();
		++ppCur;
	}
	while(ppCur != xm_vecItems.GetBegin()){
		--ppCur;
		(*ppCur)->Unlock();
	}
	return true;
}
