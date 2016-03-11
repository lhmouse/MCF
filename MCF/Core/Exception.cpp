// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Exception.hpp"

namespace MCF {

void Exception::RethrowCurrentNestedExceptionIfExists(){
	const auto pCurrentException = std::current_exception();
	if(!pCurrentException){
		return;
	}

	std::exception_ptr pNestedException;
	try {
		std::rethrow_exception(pCurrentException);
	} catch(Exception &e){
		pNestedException = e.GetNestedException();
	} catch(...){
		//
	}
	if(pNestedException){
		std::rethrow_exception(pNestedException);
	}
}

Exception::~Exception(){
}

}
