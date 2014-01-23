#include <MCF/StdMCF.hpp>
#include <MCF/Core/File.hpp>
#include <cstdio>
#include <cstdlib>

unsigned int MCFMain(){
	MCF::File f;
	f.Open(L"E:\\Desktop\\123.txt", true, false, false);
	std::printf("is open = %d, err = %lu\n", (int)f.IsOpen(), ::GetLastError());

	const auto fsize = f.GetSize();
	std::printf("file size = %llu\n", fsize);
	char *buffer = new char[fsize];

	unsigned long to_read;
	unsigned long long offset;
	unsigned long cb_read;

	try {
		to_read = fsize;
		offset = 0;
		std::printf("reading %lu bytes from offset %llu...\n", to_read, offset);
		cb_read = f.Read(buffer, offset, to_read, []{ std::puts("-- throw in async proc"); throw 5; });
		std::printf("%lu bytes read, err = %lu\n", cb_read, ::GetLastError());
	} catch(int e){
		std::printf("exception caught, e = %d\n", e);
	}

	delete[] buffer;

	return 0;
}
