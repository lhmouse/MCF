#include <MCF/StdMCF.hpp>
#include <MCF/Core/SharedHandle.hpp>
#include <MCF/Language/Notation.hpp>
using namespace MCF;

struct deleter {
	constexpr int *operator()() const noexcept {
		return nullptr;
	}
	void operator()(int *p) const noexcept {
		delete p;
	}
};

template class SharedHandle<deleter>;

extern "C" unsigned int MCFMain() noexcept {
	SharedHandle<deleter> p;
	p.Reset(new int(1));
	p.Reset(new int(2));
	p.Reset(new int(3));

	Notation n;
	auto res = n.Parse(LR"(
		ZZZ {
			z = z
			y = y
			a = a
			b = b
		}
		YYY {
			z = z
			y = y
			a = a
			b = b
		}
		AAA {
			z = z
			y = y
			a = a
			b = b
		}
		BBB {
			z = z
			y = y
			a = a
			b = b
		}
	)"_wso);
	std::printf("err = %d\n%ls\n", res.first, n.Export().GetStr());
	return 0;
}
