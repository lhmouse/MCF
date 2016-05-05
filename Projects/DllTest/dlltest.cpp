#include <windows.h>
#include <cstdio>

#define DYNAMIC_LOAD	0
#define DYNAMIC_UNLOAD	1

#if DYNAMIC_LOAD
int (__stdcall *dlltest)(int, int) noexcept = nullptr;
#else
extern "C" __declspec(dllimport) int __stdcall dlltest(int a, int b) noexcept;
#endif

DWORD __stdcall thread_proc(void *) noexcept {
	std::printf("in thread_proc() - dlltest(123, 456) = %d\n", ::dlltest(123, 456));
}

int main(void){
#if DYNAMIC_LOAD
	HMODULE dll = ::LoadLibraryW(L".dll-Debug32.dll");
	if(!dll){
		std::printf("LoadLibraryW() failed with error %lu\n", ::GetLastError());
		exit(1);
	}
	dlltest = (int (__stdcall *)(int, int))::GetProcAddress(dll, "dlltest@8");
	if(!dlltest){
		std::printf("GetProcAddress() failed with error %lu\n", ::GetLastError());
		exit(1);
	}
#endif
	std::printf("in main() - dlltest(654, 321) = %d\n", ::dlltest(654, 321));

	HANDLE thread = ::CreateThread(0, 0, thread_proc, 0, 0, 0);
	if(thread){
		::WaitForSingleObject(thread, INFINITE);
		::CloseHandle(thread);
	}
#if DYNAMIC_LOAD
#	if DYNAMIC_UNLOAD
	::FreeLibrary(dll);
#	endif
#endif
}
