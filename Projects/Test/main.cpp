extern "C" unsigned MCFMain(){
	auto p = new char[100];
	// p[-1] = 0;
	delete[] p;
	return 0;
}
