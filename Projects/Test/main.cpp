#include <MCF/Containers/Vector.hpp>
#include <MCF/Core/String.hpp>

template class MCF::Vector<MCF::AnsiString>;

extern "C" unsigned MCFMain(){
	MCF::Vector<MCF::AnsiString> v1, v2;
	for(int i = 0; i < 10; ++i){
		char temp[64];
		std::sprintf(temp, "---------------------------------- hello %d", i);
		v1.Push(temp);
		std::sprintf(temp, "---------------------------------- world %d", i);
		v2.Push(temp);
	}

	try {
		// v1.InsertRange(v1.GetData() + 5, v2.GetBegin() + 2, v2.GetBegin() + 6);
		v1.Insert(v1.GetData() + 5, "meow");
	} catch(std::exception &e){
		std::printf("exception: what = %s\n", e.what());
	}

	for(auto &s : v1){
		std::printf("v1: %s\n", s.GetStr());
	}
	return 0;
}
