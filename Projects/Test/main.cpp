extern "C" wchar_t *_MCFCRT_wcpcpy(wchar_t *__restrict__ __dst, const wchar_t *__restrict__ __src) noexcept;

wchar_t s[2][53];

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	__builtin_memset(s[0], 0xAA, sizeof(s[0]));
	__builtin_memset(s[1], 0x01, sizeof(s[1]));
	s[1][47] = 0;
	_MCFCRT_wcpcpy(s[0] + 3, s[1]);
	return 0;
}
