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
bool Transaction::Commit() const {
	auto vCur = xm_deqItems.GetFirstCursor();
	try {
		while(vCur){
			if(!(*vCur)->Lock()){
				break;
			}
			++vCur;
		}
	} catch(...){
		while(--vCur){
			(*vCur)->Unlock();
		}
		throw;
	}
	if(vCur){
		while(--vCur){
			(*vCur)->Unlock();
		}
		return false;
	}

	for(vCur = xm_deqItems.GetFirstCursor(); vCur; ++vCur){
		(*vCur)->Commit();
	}
	for(vCur = xm_deqItems.GetLastCursor(); vCur; --vCur){
		(*vCur)->Unlock();
	}
	return true;
}
