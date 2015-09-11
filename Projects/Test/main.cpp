extern "C" unsigned MCFMain(){
	try {
		throw 123;
	} catch(int e){
		__builtin_printf("exception: e = %d\n", e);
	}
	return 0;
}
