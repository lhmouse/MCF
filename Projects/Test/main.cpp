#include <thread>
#include <mutex>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	std::mutex m;
	std::thread t;

	return 0;
}
