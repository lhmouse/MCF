#include <windows.h>
#include <cstdio>

int (__stdcall *dlltest)(int, int) noexcept = nullptr;

DWORD __stdcall thread_proc(void *) noexcept {
	std::printf("in thread_proc() - dlltest(123, 456) = %d\n", ::dlltest(123, 456));
}

int main(){
	HMODULE dll = ::LoadLibraryW(L".Debug32.dll");
	if(!dll){
		std::printf("LoadLibraryW() failed with error %lu\n", ::GetLastError());
		exit(1);
	}
	dlltest = (int (__stdcall *)(int, int))::GetProcAddress(dll, "dlltest@8");
	if(!dlltest){
		std::printf("GetProcAddress() failed with error %lu\n", ::GetLastError());
		exit(1);
	}

	std::printf("in main() - dlltest(654, 321) = %d\n", ::dlltest(654, 321));

	HANDLE thread = ::CreateThread(0, 0, thread_proc, 0, 0, 0);
	if(thread){
		::WaitForSingleObject(thread, INFINITE);
		::CloseHandle(thread);
	}
}
