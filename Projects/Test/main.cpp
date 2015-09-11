extern "C" unsigned MCFMain(){
	auto p = new int[5];
	p[-1] = 0;
	delete[] p;
	return 0;
}
