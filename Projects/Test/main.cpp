extern "C" unsigned MCFMain(){
	auto p = new char[5];
	p[11] = 123;
	__builtin_printf("c = %02X\n", (unsigned char)p[2]);
	delete[] p;
	return 0;
}
