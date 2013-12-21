// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

// MCFMain() 函数在该文件中拥有 C 链接性。

namespace MCF {
	namespace __MCF {
		extern unsigned int CPPMainHack();
	}
}

extern "C" unsigned int __cdecl MCFMain(){
	// 如果用户定义了具有 C++ 链接性的 MCFMain() 函数，
	// 我们通过这个跳板调用对应的具有 C 链接性的 MCFMain() 函数。
	// 由于两个函数仅仅链接性不同，我们不能在同一个源文件中进行这个操作。
	return ::MCF::__MCF::CPPMainHack();
}
