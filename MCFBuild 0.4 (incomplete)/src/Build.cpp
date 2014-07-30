// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "../MCF/Serialization/Serdes.hpp"
using namespace MCFBuild;

namespace {


}

namespace MCFBuild {

void Build(){
	MCF::VList<int> l{0,1,2,3,4,5,6,7,8,9};
	for(auto p = l.GetBegin(); p; p = p->GetNext()){
		std::printf("%d ", p->GetElement());
	}
	std::putchar('\n');
	l.Reverse();
	for(auto p = l.GetBegin(); p; p = p->GetNext()){
		std::printf("%d ", p->GetElement());
	}
	std::putchar('\n');
}

}
