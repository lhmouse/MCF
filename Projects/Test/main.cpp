extern "C" unsigned MCFMain(){
	volatile double f = 5.9;
	double d = __builtin_floor(f);
	__builtin_printf("d = %f\n", d);
	return 0;
}
