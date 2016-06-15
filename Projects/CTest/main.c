#include <mcfgthread/gthread.h>

unsigned _MCFCRT_Main(void){
	__gthread_recursive_mutex_t m = { 0 };
	__gthread_recursive_mutex_lock(&m);
	__gthread_recursive_mutex_unlock(&m);
	return 0;
}
