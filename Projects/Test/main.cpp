#include <MCF/StdMCF.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCFCRT/env/module.h>
#include <MCFCRT/ext/itoa.h>

extern "C" unsigned _MCFCRT_Main(){
	MCF::StandardOutputStream os;
	::_MCFCRT_ModuleSectionInfo info;
	if(::_MCFCRT_EnumerateFirstModuleSection(&info)){
		do {
			char str[64];
			os.Put("Section: name = ", 16);
			auto end = (char *)memchr(info.__achName, 0, sizeof(info.__achName));
			if(!end){
				end = info.__achName + 8;
			}
			os.Put(info.__achName, (std::size_t)(end - info.__achName));
			os.Put(", raw_size = ", 13);
			end = ::_MCFCRT_itoa0X(str, info.__uRawSize, sizeof(info.__uRawSize) * 2);
			os.Put(str, (std::size_t)(end - str));
			os.Put(", address = ", 12);
			end = ::_MCFCRT_itoa0X(str, (std::uintptr_t)info.__pBase, sizeof(info.__pBase) * 2);
			os.Put(str, (std::size_t)(end - str));
			os.Put(", size = ", 9);
			end = ::_MCFCRT_itoa0X(str, (std::uintptr_t)info.__uSize, sizeof(info.__uSize) * 2);
			os.Put(str, (std::size_t)(end - str));
			os.Put('\n');
		} while(::_MCFCRT_EnumerateNextModuleSection(&info));
	}
	return 0;
}
