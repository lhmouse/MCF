#include <MCF/Containers/RingQueue.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	RingQueue<int> q;
	q.Append({0,1,2,3,4,5,6,7,8,9});
	q.Shift(3);
	return 0;
}
