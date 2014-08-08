AUTOMAKE_OPTIONS=foreign subdir-objects

AM_CFLAGS=-std=c11 -Wall -Wextra -Wsign-conversion -Wsuggest-attribute=noreturn -pipe	\
	-mfpmath=sse,387 -march=core2 -masm=intel
AM_CXXFLAGS=-std=c++14 -Wall -Wextra -Wsign-conversion -Wsuggest-attribute=noreturn -pipe	\
	-mfpmath=sse,387 -march=core2 -masm=intel -Wnoexcept

noinst_LIBRARIES=	\
	src/libmcflite.a

src_libmcflite_a_SOURCES=\
	../MCFCRT/stdc/stdlib/atexit.c			\
	../MCFCRT/stdc/stdlib/malloc.c			\
	../MCFCRT/stdc/stdlib/realloc.c			\
	../MCFCRT/stdc/stdlib/calloc.c			\
	../MCFCRT/stdc/stdlib/free.c			\
	../MCFCRT/ext/wcscpyout.c				\
	../MCFCRT/env/bail.c					\
	../MCFCRT/env/module.c					\
	../MCFCRT/env/last_error.c				\
	../MCFCRT/env/fenv.c					\
	../MCFCRT/env/mingw_hacks.c				\
	../MCFCRT/env/avl_tree.c				\
	../MCFCRT/env/thread.c					\
	../MCFCRT/env/heap.c					\
	../MCFCRT/env/heap_dbg.c				\
	../MCFCRT/env/cxa_throw.cpp				\
	../MCFCRT/exe/exe_startup.c				\
	../MCF/Core/String.cpp					\
	../MCF/Core/File.cpp					\
	../MCF/Core/Utf8TextFile.cpp			\
	../MCF/Core/System.cpp					\
	../MCF/Core/Time.cpp					\
	../MCF/Thread/Thread.cpp				\
	../MCF/Thread/CriticalSection.cpp		\
	../MCF/Thread/Semaphore.cpp				\
	../MCF/Thread/ReaderWriterLock.cpp		\
	../MCF/Language/Notation.cpp			\
	../MCF/Hash/Sha256.cpp					\
	../MCF/Containers/StreamBuffer.cpp

src_libmcflite_a_CFLAGS=\
	$(AM_CFLAGS) -D__MCF_CRT_NO_DLMALLOC

bin_PROGRAMS=	\
	MCFBuild

MCFBuild_LDFLAGS=	\
	-Wall -Wextra -static -fwhole-program -nostdlib	\
	-Wl,-e__MCF_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import	\
	-Wl,--wrap=atexit,--wrap=malloc,--wrap=realloc,--wrap=calloc,--wrap=free,--wrap=__cxa_throw

MCFBuild_DEPENDENCIES=	\
	src/libmcflite.a

MCFBuild_LDADD=	\
	-Lsrc -lmcflite -lstdc++ -lmingwex -lmingw32 -lgcc -lgcc_eh -lmcflite -lmsvcrt -lkernel32 -luser32

MCFBuild_SOURCES=	\
	src/ConsoleOutput.cpp		\
	src/DependencyDatabase.cpp	\
	src/FileSystem.cpp			\
	src/JobScheduler.cpp		\
	src/Localization.cpp		\
	src/MCFMain.cpp				\
	src/Model.cpp				\
	src/ProjectFile.cpp			\
	src/Shell.cpp				\
	src/Build.cpp
