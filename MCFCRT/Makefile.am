ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = no-exeext foreign subdir-objects
AM_CPPFLAGS = -Wall -Wextra -pedantic -pedantic-errors -Werror -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wdouble-promotion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2	\
	-pipe -mfpmath=both -march=core2 -mtune=intel -mno-stack-arg-probe -masm=intel -ffreestanding
AM_CFLAGS = -include __pch.h -std=c11 -Wstrict-prototypes
AM_CXXFLAGS = -include __pch.hpp -std=c++17 -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -fnothrow-opt

## I think you GNU people should just STFU and stop confusing the linker.
EXEEXT =

BUILT_SOURCES = \
	__pch.h	\
	__pch.h.gch	\
	__pch.hpp	\
	__pch.hpp.gch

CLEANFILES = \
	__pch.h	\
	__pch.h.gch	\
	__pch.hpp	\
	__pch.hpp.gch

__pch.h: ${srcdir}/src/env/_crtdef.h
	echo "#include \"$$(cygpath -m "$<" || readlink -f "$<")\"" > $@

__pch.h.gch: __pch.h
	${CC} -x c-header @DEFS@ ${AM_CPPFLAGS} ${CPPFLAGS} $$(echo "" "${AM_CFLAGS}" | sed "s/-include __pch\\.h//") ${CFLAGS} -Wno-error $< -o $@

__pch.hpp: ${srcdir}/src/env/_crtdef.hpp
	echo "#include \"$<\"" > $@

__pch.hpp.gch: __pch.hpp
	${CXX} -x c++-header @DEFS@ ${AM_CPPFLAGS} ${CPPFLAGS} $$(echo "" "${AM_CXXFLAGS}" | sed "s/-include __pch\\.hpp//") ${CXXFLAGS} $< -o $@

noinst_HEADERS = \
	src/stdc/math/_fpu.h	\
	src/stdc/math/_sse2.h	\
	src/stdc/math/_sse3.h	\
	src/stdc/string/_memcpy_impl.h	\
	src/stdc/string/_sse2.h	\
	src/stdc/string/_ssse3.h

pkginclude_HEADERS = \
	src/mcfcrt.h	\
	src/mcfwin.h	\
	src/gthread.h	\
	src/c11thread.h

pkginclude_predir = ${pkgincludedir}/pre
pkginclude_pre_HEADERS = \
	src/pre/dll.h	\
	src/pre/exe.h	\
	src/pre/module.h	\
	src/pre/tls.h	\
	src/pre/c11thread_tls.c	\
	src/pre/gthread_tls.c

pkginclude_envdir = ${pkgincludedir}/env
pkginclude_env_HEADERS = \
	src/env/_crtdef.h	\
	src/env/_crtdef.hpp	\
	src/env/xassert.h	\
	src/env/expect.h	\
	src/env/offset_of.h	\
	src/env/pp.h	\
	src/env/_fpu.h	\
	src/env/_seh_top.h	\
	src/env/_nt_timeout.h	\
	src/env/_mopthread.h	\
	src/env/_tls_common.h	\
	src/env/_atexit_queue.h	\
	src/env/_make_constant.h	\
	src/env/inline_mem.h	\
	src/env/argv.h	\
	src/env/avl_tree.h	\
	src/env/bail.h	\
	src/env/c11thread.h	\
	src/env/clocks.h	\
	src/env/condition_variable.h	\
	src/env/gthread.h	\
	src/env/heap.h	\
	src/env/heap_debug.h	\
	src/env/last_error.h	\
	src/env/mcfwin.h	\
	src/env/mutex.h	\
	src/env/once_flag.h	\
	src/env/standard_streams.h	\
	src/env/thread.h	\
	src/env/crt_module.h

pkginclude_extdir = ${pkgincludedir}/ext
pkginclude_ext_HEADERS = \
	src/ext/alloca.h	\
	src/ext/div64.h	\
	src/ext/itoa.h	\
	src/ext/itow.h	\
	src/ext/atoi.h	\
	src/ext/wtoi.h	\
	src/ext/mul64.h	\
	src/ext/random.h	\
	src/ext/stpcpy.h	\
	src/ext/stppcpy.h	\
	src/ext/thunk.h	\
	src/ext/utf.h	\
	src/ext/wcpcpy.h	\
	src/ext/wcppcpy.h	\
	src/ext/rawmemchr.h	\
	src/ext/rawwmemchr.h	\
	src/ext/rep_movs.h	\
	src/ext/rep_stos.h	\
	src/ext/rep_cmps.h	\
	src/ext/rep_scas.h

mcfcrt_pre_sources = \
	src/pre/module.c	\
	src/pre/atexit.c	\
	src/pre/at_quick_exit.c	\
	src/pre/tls.c	\
	src/pre/c11thread_tls.c	\
	src/pre/gthread_tls.c	\
	src/pre/__cxa_atexit.c	\
	src/pre/__cxa_thread_atexit.c	\
	src/pre/_libsupcxx_cleanup.cpp	\
	src/pre/_pei386_runtime_relocator.c

mcfcrt_sources = \
	src/mcfcrt.c	\
	src/env/_fpu.c	\
	src/env/_nt_timeout.c	\
	src/env/_seh_top.c	\
	src/env/_mopthread.c	\
	src/env/_tls_common.c	\
	src/env/xassert.c	\
	src/env/argv.c	\
	src/env/avl_tree.c	\
	src/env/bail.c	\
	src/env/c11thread.c	\
	src/env/clocks.c	\
	src/env/condition_variable.c	\
	src/env/gthread.c	\
	src/env/heap.c	\
	src/env/heap_debug.c	\
	src/env/last_error.c	\
	src/env/mutex.c	\
	src/env/once_flag.c	\
	src/env/standard_streams.c	\
	src/env/thread.c	\
	src/env/crt_module.c	\
	src/ext/itoa.c	\
	src/ext/itow.c	\
	src/ext/atoi.c	\
	src/ext/wtoi.c	\
	src/ext/random.c	\
	src/ext/stpcpy.c	\
	src/ext/stppcpy.c	\
	src/ext/thunk.c	\
	src/ext/utf.c	\
	src/ext/wcpcpy.c	\
	src/ext/wcppcpy.c	\
	src/ext/rawmemchr.c	\
	src/ext/rawwmemchr.c	\
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
	src/stdc/math/fma.c	\
	src/stdc/math/cbrt.c	\
	src/stdc/math/signbit.c	\
	src/stdc/math/copysign.c	\
	src/stdc/stdlib/abort.c	\
	src/stdc/stdlib/abs.c	\
	src/stdc/stdlib/calloc.c	\
	src/stdc/stdlib/free.c	\
	src/stdc/stdlib/malloc.c	\
	src/stdc/stdlib/realloc.c	\
	src/stdc/string/_memcpy_impl.c	\
	src/stdc/string/memchr.c	\
	src/stdc/string/memcmp.c	\
	src/stdc/string/memcpy.c	\
	src/stdc/string/memmove.c	\
	src/stdc/string/memset.c	\
	src/stdc/string/strchr.c	\
	src/stdc/string/strcmp.c	\
	src/stdc/string/strcpy.c	\
	src/stdc/string/strlen.c	\
	src/stdc/string/strncmp.c	\
	src/stdc/wchar/wcschr.c	\
	src/stdc/wchar/wcscmp.c	\
	src/stdc/wchar/wcscpy.c	\
	src/stdc/wchar/wcslen.c	\
	src/stdc/wchar/wcsncmp.c	\
	src/stdc/wchar/wmemchr.c	\
	src/stdc/wchar/wmemcmp.c	\
	src/stdc/wchar/wmemcpy.c	\
	src/stdc/wchar/wmemmove.c	\
	src/stdc/wchar/wmemset.c	\
	src/stdc/stdlib/_Exit.c	\
	src/stdc/stdlib/exit.c	\
	src/stdc/stdlib/quick_exit.c

bin_PROGRAMS = \
	MCFCRT-2.dll

MCFCRT_2_dll_SOURCES = \
	src/dll_startup.c	\
	${mcfcrt_sources}

MCFCRT_2_dll_LDFLAGS = \
	-shared -static -nostdlib	\
	-Wl,-e@__MCFCRT_DllStartup,-subsystem,windows,--disable-runtime-pseudo-reloc,--disable-auto-import	\
	-Wl,--export-all-symbols,--exclude-symbols,@__MCFCRT_DllStartup,--exclude-libs,ALL	\
	-Wl,--disable-stdcall-fixup,--enable-auto-image-base,--out-implib,libMCFCRT.dll.a

MCFCRT_2_dll_LDADD = \
	-lkernel32 -lntdll

lib_LIBRARIES = \
	libMCFCRT-pre-exe.a	\
	libMCFCRT-pre-dll.a	\
	libMCFCRT.dll.a	\
	libMCFCRT.a

libMCFCRT_pre_exe_a_SOURCES = \
	src/pre/exe.c	\
	${mcfcrt_pre_sources}

libMCFCRT_pre_dll_a_SOURCES = \
	src/pre/dll.c	\
	${mcfcrt_pre_sources}

libMCFCRT_dll_a_SOURCES =
libMCFCRT.dll.a: MCFCRT-2.dll

libMCFCRT_a_SOURCES = \
	${mcfcrt_sources}
