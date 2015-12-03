#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/hooks.h>
#include <iostream>
#include <string>

extern "C" unsigned MCFMain(){
	::__MCF_CRT_OnException = [](void *ptr, const std::type_info &ti, const void *ret){
		std::cout <<"ptr = " <<ptr <<", type = " <<ti.name() <<", ret = " <<ret <<std::endl;
	};

	try {
		throw std::string("hello");
	} catch(...){
		std::cout <<"unknown exception caught!" <<std::endl;
	}
	return 0;
}
