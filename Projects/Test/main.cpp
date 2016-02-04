#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/MinMax.hpp>
#include <iostream>


struct foo {
    int i;

    explicit foo(int x)
        : i(x)
    {
    }
    foo(const foo &r)
        : i(r.i)
    {
    }
    ~foo(){
        i = -1;
    }

    friend bool operator<(const foo &l, const foo &r){
        return l.i < r.i;
    }
};

extern "C" unsigned MCFCRT_Main(){
    const auto &r = MCF::Max(foo(3), foo(1), foo(2));
    std::cout <<"r.i = " <<r.i <<std::endl;
	return 0;
}
