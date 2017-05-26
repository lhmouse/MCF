// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_MCFCRT_H_
#define __MCFCRT_MCFCRT_H_

#include "env/_crtdef.h"

#include "env/argv.h"
#include "env/avl_tree.h"
#include "env/bail.h"
#include "env/clocks.h"
#include "env/condition_variable.h"
#include "env/xassert.h"
#include "env/crt_module.h"
#include "env/expect.h"
#include "env/heap.h"
#include "env/heap_dbg.h"
#include "env/hooks.h"
#include "env/inline_mem.h"
#include "env/last_error.h"
#include "env/mutex.h"
#include "env/offset_of.h"
#include "env/once_flag.h"
#include "env/pp.h"
#include "env/standard_streams.h"
#include "env/thread.h"

#include "ext/alloca.h"
#include "ext/atoi.h"
#include "ext/wtoi.h"
#include "ext/itoa.h"
#include "ext/itow.h"
#include "ext/mul64.h"
#include "ext/div64.h"
#include "ext/random.h"
#include "ext/rawmemchr.h"
#include "ext/rawwmemchr.h"
#include "ext/rep_movs.h"
#include "ext/rep_stos.h"
#include "ext/repz_cmps.h"
#include "ext/repnz_cmps.h"
#include "ext/repz_scas.h"
#include "ext/repnz_scas.h"
#include "ext/stpcpy.h"
#include "ext/wcpcpy.h"
#include "ext/utf.h"
#include "ext/thunk.h"

#include "pre/module.h"
#include "pre/exe.h"
#include "pre/dll.h"
#include "pre/tls.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_InitRecursive(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_UninitRecursive(void) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
