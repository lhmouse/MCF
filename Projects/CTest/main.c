#include <mcfgthread/gthread.h>

__gthread_recursive_mutex_t m;

unsigned _MCFCRT_Main(void){
	__gthread_recursive_mutex_lock(&m);
	__gthread_recursive_mutex_unlock(&m);
	return 0;
}
