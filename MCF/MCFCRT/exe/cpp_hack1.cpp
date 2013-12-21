// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

// MCFMain() 函数在该文件中拥有 C++ 链接性。

extern unsigned int __cdecl MCFMain();

namespace MCF {
	namespace __MCF {
		extern unsigned int CPPMainHack(){
			return MCFMain();
		}
	}
}
