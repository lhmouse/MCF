#include <MCF/StdMCF.hpp>
#include <MCF/Languages/MNotation.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	MNotation n;
	n.Parse(LR"___(
		foo {
			child1 = this is value 1
		}
		bar = foo {
			child2 = this is value 2
		}
	)___"_wso);

	auto fchild1 = n.Get(L"foo"_wso)->Get().second.Get(L"child1"_wso);
	auto bchild1 = n.Get(L"bar"_wso)->Get().second.Get(L"child1"_wso);

	std::printf("%ls\n---------\nfoo.child1 = %p\nbar.child1 = %p\n", n.Export().GetStr(), fchild1, bchild1);

	return 0;
}
