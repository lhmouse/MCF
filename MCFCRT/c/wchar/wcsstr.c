// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern size_t wcslen(const wchar_t *s);

extern void *malloc(size_t cb);
extern void free(void *ptr);
extern int wmemcmp(const wchar_t *p1, const wchar_t *p2, size_t cnt);

wchar_t *wcsstr(const wchar_t *s1, const wchar_t *s2){
	// http://en.wikipedia.org/wiki/Knuth–Morris–Pratt_algorithm
	const wchar_t *s = s1;
	const size_t slen = wcslen(s1);
	const wchar_t *w = s2;
	const size_t wlen = wcslen(s2);

	size_t t_sm[256];

	size_t *t;
	if(wlen <= sizeof(t_sm) / sizeof(t_sm[0])){
		t = t_sm;
	} else {
		t = malloc(wlen * sizeof(size_t));
		if(!t){
			const wchar_t *till = s  + (slen - wlen);
			const wchar_t *p = s;
			do {
				if(wmemcmp(p, w, wlen) == 0){
					return (wchar_t *)p;
				}
			} while(++p != till);
			return NULL;
		}
	}

	size_t pos = 2;
	size_t cnd = 0;

	t[0] = (size_t)-1;
	t[1] = 0;

	while(pos < wlen){
		if(w[pos - 1] == w[cnd]){
			++cnd;
			t[pos] = cnd;
			++pos;
		} else if(cnd != 0){
			cnd = t[cnd];
		} else {
			t[pos] = 0;
			++pos;
		}
	}

	size_t m = 0;
	size_t i = 0;

	wchar_t *ret = NULL;

	while(m + i < slen){
		if(w[i] == s[m + i]){
			if(i == wlen - 1){
				ret = (wchar_t *)(s + m);
				break;
			}
			++i;
		} else {
			const size_t entry = t[i];
			m += i - entry;
			if(entry != (size_t)-1){
				i = entry;
			} else {
				i = 0;
			}
		}
	}

	if(t != t_sm){
		free(t);
	}

	return ret;
}
