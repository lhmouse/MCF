#include <MCF/StdMCF.hpp>
using namespace MCF;

void meow(){
    __builtin_puts("-- meow()");
}
void bark(){
    __builtin_puts("-- bark()");
}

struct foo {
    foo(){
        __builtin_puts("foo()");
        std::atexit(meow);
    }
    ~foo(){
        __builtin_puts("~foo()");
    }
};
struct bar {
    bar(){
        __builtin_puts("bar()");
        std::atexit(bark);
    }
    ~bar(){
        __builtin_puts("~bar()");
    }
};

foo f;
bar b;

extern "C" unsigned int MCFMain() noexcept {
	return 0;
}
