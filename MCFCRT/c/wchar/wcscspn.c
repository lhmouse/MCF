// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

extern wchar_t *wcschr(const wchar_t *s, wchar_t ch);

size_t wcscspn(const wchar_t *s1, const wchar_t *s2){
	const wchar_t *rp = s1;
	for(;;){
		const wchar_t ch = *rp;
		if(ch == 0){
			break;
		}
		if(wcschr(s2, ch)){
			break;
		}
		++rp;
	}
	return (size_t)(rp - s1);
}
