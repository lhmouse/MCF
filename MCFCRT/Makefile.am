ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = no-exeext foreign subdir-objects
AM_CPPFLAGS = -Wall -Wextra -pedantic -pedantic-errors -Werror -Wno-error=unused-parameter	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel
AM_CFLAGS = -std=c11 -Wstrict-prototypes
AM_CXXFLAGS = -std=c++14 -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -fnothrow-opt

## I think you GNU people should just STFU and stop confusing the linker.
EXEEXT =

pkginclude_startupdir = $(pkgincludedir)/startup
pkginclude_startup_HEADERS =	\
	src/startup/generic.h	\
	src/startup/dll.h	\
	src/startup/exe.h

pkginclude_envdir = $(pkgincludedir)/env
pkginclude_env_HEADERS =	\
	src/ext/alloca.h	\
	src/ext/assert.h	\
	src/ext/div64.h	\
	src/ext/expect.h	\
	src/ext/itoa.h	\
	src/ext/itow.h	\
	src/ext/mul64.h	\
	src/ext/offset_of.h	\
	src/ext/random.h	\
	src/ext/stpcpy.h	\
	src/ext/thunk.h	\
	src/ext/utf.h	\
	src/ext/wcpcpy.h	\
	src/env/_c11thread_inl.h	\
	src/env/_crtdef.h	\
	src/env/_gthread_inl.h	\
	src/env/argv.h	\
	src/env/avl_tree.h	\
	src/env/bail.h	\
	src/env/c11thread.h	\
	src/env/clocks.h	\
	src/env/condition_variable.h	\
	src/env/fenv.h	\
	src/env/gthread.h	\
	src/env/heap.h	\
	src/env/heap_dbg.h	\
	src/env/hooks.h	\
	src/env/last_error.h	\
	src/env/mcfwin.h	\
	src/env/module.h	\
	src/env/mutex.h	\
	src/env/once_flag.h	\
	src/env/standard_streams.h	\
	src/env/thread.h

pkginclude_extdir = $(pkgincludedir)/ext
pkginclude_ext_HEADERS =	\
	src/ext/alloca.h	\
	src/ext/assert.h	\
	src/ext/div64.h	\
	src/ext/expect.h	\
	src/ext/itoa.h	\
	src/ext/itow.h	\
	src/ext/mul64.h	\
	src/ext/offset_of.h	\
	src/ext/random.h	\
	src/ext/stpcpy.h	\
	src/ext/thunk.h	\
	src/ext/utf.h	\
	src/ext/wcpcpy.h

mcfcrt_sources =	\
	src/startup/generic.c	\
	src/ext/assert.c	\
	src/ext/itoa.c	\
	src/ext/itow.c	\
	src/ext/random.c	\
	src/ext/stpcpy.c	\
	src/ext/thunk.c	\
	src/ext/utf.c	\
	src/ext/wcpcpy.c	\
	src/env/_nt_timeout.c	\
	src/env/_seh_top.c	\
	src/env/argv.c	\
	src/env/avl_tree.c	\
	src/env/bail.c	\
	src/env/c11thread.c	\
	src/env/clocks.c	\
	src/env/condition_variable.c	\
	src/env/fenv.c	\
	src/env/gthread.c	\
	src/env/heap.c	\
	src/env/heap_dbg.c	\
	src/env/hooks.c	\
	src/env/last_error.c	\
	src/env/module.c	\
	src/env/mutex.c	\
	src/env/once_flag.c	\
	src/env/standard_streams.c	\
	src/env/thread.c	\
	src/stdc/math/acos.c	\
	src/stdc/math/asin.c	\
	src/stdc/math/atan.c	\
	src/stdc/math/atan2.c	\
	src/stdc/math/ceil.c	\
	src/stdc/math/cos.c	\
	src/stdc/math/exp.c	\
	src/stdc/math/exp2.c	\
	src/stdc/math/expm1.c	\
	src/stdc/math/fabs.c	\
	src/stdc/math/fdim.c	\
	src/stdc/math/floor.c	\
	src/stdc/math/fmax.c	\
	src/stdc/math/fmin.c	\
	src/stdc/math/fmod.c	\
	src/stdc/math/frexp.c	\
	src/stdc/math/hypot.c	\
	src/stdc/math/ilogb.c	\
	src/stdc/math/ldexp.c	\
	src/stdc/math/llround.c	\
	src/stdc/math/log.c	\
	src/stdc/math/log10.c	\
	src/stdc/math/log1p.c	\
	src/stdc/math/log2.c	\
	src/stdc/math/logb.c	\
	src/stdc/math/lround.c	\
	src/stdc/math/modf.c	\
	src/stdc/math/pow.c	\
	src/stdc/math/remainder.c	\
	src/stdc/math/remquo.c	\
	src/stdc/math/round.c	\
	src/stdc/math/scalbln.c	\
	src/stdc/math/scalbn.c	\
	src/stdc/math/sin.c	\
	src/stdc/math/sqrt.c	\
	src/stdc/math/tan.c	\
	src/stdc/math/trunc.c	\
	src/stdc/math/_constants.c	\
	src/stdc/stdlib/abort.c	\
	src/stdc/stdlib/abs.c	\
	src/stdc/stdlib/atexit.c	\
	src/stdc/stdlib/calloc.c	\
	src/stdc/stdlib/free.c	\
	src/stdc/stdlib/malloc.c	\
	src/stdc/stdlib/realloc.c	\
	src/stdc/string/memchr.c	\
	src/stdc/string/memcmp.c	\
	src/stdc/string/memcpy.c	\
	src/stdc/string/memmove.c	\
	src/stdc/string/memset.c	\
	src/stdc/string/strcmp.c	\
	src/stdc/string/strcpy.c	\
	src/stdc/string/strlen.c	\
	src/stdc/string/strncmp.c	\
	src/stdc/wchar/wcscmp.c	\
	src/stdc/wchar/wcscpy.c	\
	src/stdc/wchar/wcslen.c	\
	src/stdc/wchar/wcsncmp.c	\
	src/stdc/wchar/wmemchr.c	\
	src/stdc/wchar/wmemcmp.c	\
	src/stdc/wchar/wmemcpy.c	\
	src/stdc/wchar/wmemmove.c	\
	src/stdc/wchar/wmemset.c

bin_PROGRAMS =	\
	MCFCRT-9.dll

MCFCRT_9_dll_SOURCES =	\
	${mcfcrt_sources}	\
	src/startup/_cpp_runtime.c	\
	src/startup/dll.c

MCFCRT_9_dll_DEPENDENCIES =	\
	libMCFCRT-pre.a

MCFCRT_9_dll_LDFLAGS =	\
	-shared -static -nostdlib	\
	-Wl,-e@__MCFCRT_DllStartup,-subsystem,windows,--disable-runtime-pseudo-reloc,--disable-auto-import	\
	-Wl,--export-all-symbols,--exclude-symbols,@__MCFCRT_DllStartup	\
	-Wl,--disable-stdcall-fixup,--enable-auto-image-base,--out-implib,libMCFCRT.dll.a,--exclude-libs,libMCF-pre.a
##	-Wl,--whole-archive,-lsupc++,-lgcc,-lgcc_eh,--no-whole-archive

MCFCRT_9_dll_LDADD =	\
	-lmingwex -lmsvcrt -lkernel32 -lntdll

lib_LIBRARIES =	\
	libMCFCRT-pre.a	\
	libMCFCRT.dll.a	\
	libMCFCRT.a

libMCFCRT_pre_a_SOURCES =	\
	src/startup/_cpp_runtime.c	\
	src/startup/dll.c	\
	src/startup/exe.c

libMCFCRT.dll.a: MCFCRT-9.dll

##libMCFCRT_a_SOURCES =	\
##	${mcfcrt_sources}

## Due to Windows limitation we can't link this library statically.
libMCFCRT.a: MCFCRT-9.dll
	cp -p libMCFCRT.dll.a $@
