struct probe {
	const char *s;

	explicit probe(const char *m){
		s = m;
		__builtin_printf("probe()  : %s\n", s);
	}
	~probe(){
		__builtin_printf("~probe() : %s\n", s);
		s = (const char *)-1;
	}
	probe(const probe &) = delete;
};

probe p_gs("global static");
thread_local probe p_gt("global thread_local");

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	__builtin_printf("-- entering main()\n");
	{
		probe p_la("local automatic");
		thread_local probe p_lt("local thread_local");
		static probe p_ls("local static");
	}
	__builtin_printf("-- leaving main()\n");
	return 0;
}
