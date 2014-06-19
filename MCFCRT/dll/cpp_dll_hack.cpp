// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

// 拥有 C++ 链接性的函数定义。
extern __attribute__((__weak__)) bool MCFDll_OnProcessAttach(bool){
	return true;
}
extern __attribute__((__weak__)) void MCFDll_OnProcessDetach(bool){
}
extern __attribute__((__weak__)) void MCFDll_OnThreadAttach(){
}
extern __attribute__((__weak__)) void MCFDll_OnThreadDetach(){
}

namespace MCF {

// 拥有 C 链接性的函数定义。
extern "C" bool MCFDll_OnProcessAttach(bool bDynamic){
	return ::MCFDll_OnProcessAttach(bDynamic);
}
extern "C" void MCFDll_OnProcessDetach(bool bDynamic){
	::MCFDll_OnProcessDetach(bDynamic);
}
extern "C" void MCFDll_OnThreadAttach(){
	::MCFDll_OnThreadAttach();
}
extern "C" void MCFDll_OnThreadDetach(){
	::MCFDll_OnThreadDetach();
}

}
