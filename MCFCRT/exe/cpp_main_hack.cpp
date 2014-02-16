// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

// 拥有 C++ 链接性的 MCFMain() 函数声明。
extern unsigned int MCFMain();

namespace MCF {

// 拥有 C 链接性的 MCFMain() 函数定义。
extern "C" unsigned int MCFMain() noexcept {
	// 如果用户仅仅定义了具有 C++ 链接性的 MCFMain() 函数，
	// 则链接器会将 MCFCRT 链接到这里定义的具有 C 链接性的 MCFMain() 函数。
	// 我们通过这个“跳板”调用对应的具有 C++ 链接性的 MCFMain() 函数。
	return ::MCFMain();
}

}
