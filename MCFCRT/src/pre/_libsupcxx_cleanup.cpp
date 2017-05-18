// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

namespace __gnu_cxx {
	__attribute__((__weak__))
	void __freeres();
}

extern "C" void __MCFCRT_libsupcxx_Cleanup(){
	if(__gnu_cxx::__freeres){
		__gnu_cxx::__freeres();
	}
}
