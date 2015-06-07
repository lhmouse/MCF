#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <iostream>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	std::string s = "hello world!";
	auto pThread = Thread::Create([=]{ std::cout <<s <<std::endl; }, true);
	s.clear();

	pThread->Resume();
	pThread->Join();
	return 0;
}
