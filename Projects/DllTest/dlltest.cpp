#include <windows.h>
#include <cstdio>

extern "C" __declspec(dllimport) int __stdcall dlltest(int, int) noexcept;

DWORD __stdcall thread_proc(void *) noexcept {
	std::printf("in thread_proc() - dlltest(123, 456) = %d\n", ::dlltest(123, 456));
}

int main(){
	std::printf("in main() - dlltest(654, 321) = %d\n", ::dlltest(654, 321));

	HANDLE thread = ::CreateThread(0, 0, thread_proc, 0, 0, 0);
	if(thread){
		::WaitForSingleObject(thread, INFINITE);
		::CloseHandle(thread);
	}
}
