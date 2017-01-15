ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = no-exeext foreign subdir-objects
AM_CPPFLAGS = -Wall -Wextra -pedantic -pedantic-errors -Werror -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wsuggest-override	\
	-Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel
AM_CXXFLAGS = -include __pch.hpp -std=c++1z -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -fnothrow-opt

## I think you GNU people should just STFU and stop confusing the linker.
EXEEXT =

BUILT_SOURCES = \
	__pch.hpp	\
	__pch.hpp.gch

CLEANFILES = \
	__pch.hpp	\
	__pch.hpp.gch

__pch.hpp: ${srcdir}/src/StdMCF.hpp
	echo "#include \"$<\"" > $@

__pch.hpp.gch: __pch.hpp
	${CXX} -x c++-header @DEFS@ ${AM_CPPFLAGS} ${CPPFLAGS} $$(echo "" "${AM_CXXFLAGS}" | sed "s/-include __pch\\.hpp//") ${CXXFLAGS} $< -o $@

pkginclude_HEADERS = \
	src/StdMCF.hpp

pkginclude_Coredir = ${pkgincludedir}/Core
pkginclude_Core_HEADERS = \
	src/Core/_CheckedSizeArithmetic.hpp	\
	src/Core/_Enumerator.hpp	\
	src/Core/_ForwardedParam.hpp	\
	src/Core/_UniqueNtHandle.hpp	\
	src/Core/_KernelObjectBase.hpp	\
	src/Core/AddressOf.hpp	\
	src/Core/AlignedStorage.hpp	\
	src/Core/Array.hpp	\
	src/Core/ArrayView.hpp	\
	src/Core/Assert.hpp	\
	src/Core/Atomic.hpp	\
	src/Core/Bail.hpp	\
	src/Core/BinaryOperations.hpp	\
	src/Core/Bind.hpp	\
	src/Core/BlackWhiteHole.hpp	\
	src/Core/Converters.hpp	\
	src/Core/Clocks.hpp	\
	src/Core/Clone.hpp	\
	src/Core/Comparators.hpp	\
	src/Core/ConstructDestruct.hpp	\
	src/Core/CountLeadingTrailingZeroes.hpp	\
	src/Core/CopyMoveFill.hpp	\
	src/Core/CountOf.hpp	\
	src/Core/DeclVal.hpp	\
	src/Core/DefaultAllocator.hpp	\
	src/Core/Defer.hpp	\
	src/Core/DynamicLinkLibrary.hpp	\
	src/Core/Endian.hpp	\
	src/Core/Exception.hpp	\
	src/Core/File.hpp	\
	src/Core/FixedSizeAllocator.hpp	\
	src/Core/Function.hpp	\
	src/Core/FunctionView.hpp	\
	src/Core/Invoke.hpp	\
	src/Core/LastError.hpp	\
	src/Core/Matrix.hpp	\
	src/Core/MinMax.hpp	\
	src/Core/Noncopyable.hpp	\
	src/Core/Optional.hpp	\
	src/Core/Random.hpp	\
	src/Core/Rcnts.hpp	\
	src/Core/ReconstructOrAssign.hpp	\
	src/Core/RefWrapper.hpp	\
	src/Core/StreamBuffer.hpp	\
	src/Core/String.hpp	\
	src/Core/StringView.hpp	\
	src/Core/TupleManipulators.hpp	\
	src/Core/UniqueHandle.hpp	\
	src/Core/Uuid.hpp	\
	src/Core/Variant.hpp

pkginclude_Threaddir = ${pkgincludedir}/Thread
pkginclude_Thread_HEADERS = \
	src/Thread/_UniqueLockTemplate.hpp	\
	src/Thread/ConditionVariable.hpp	\
	src/Thread/Event.hpp	\
	src/Thread/KernelEvent.hpp	\
	src/Thread/KernelMutex.hpp	\
	src/Thread/KernelRecursiveMutex.hpp	\
	src/Thread/KernelSemaphore.hpp	\
	src/Thread/Mutex.hpp	\
	src/Thread/OnceFlag.hpp	\
	src/Thread/ReadersWriterMutex.hpp	\
	src/Thread/RecursiveMutex.hpp	\
	src/Thread/Semaphore.hpp	\
	src/Thread/Thread.hpp	\
	src/Thread/ThreadLocal.hpp	\
	src/Thread/UniqueLockBase.hpp

pkginclude_SmartPointersdir = ${pkgincludedir}/SmartPointers
pkginclude_SmartPointers_HEADERS = \
	src/SmartPointers/DefaultDeleter.hpp	\
	src/SmartPointers/IntrusivePtr.hpp	\
	src/SmartPointers/PolyIntrusivePtr.hpp	\
	src/SmartPointers/UniquePtr.hpp

pkginclude_Containersdir = ${pkgincludedir}/Containers
pkginclude_Containers_HEADERS = \
	src/Containers/_FlatContainer.hpp	\
	src/Containers/CircularQueue.hpp	\
	src/Containers/FlatMap.hpp	\
	src/Containers/FlatMultiMap.hpp	\
	src/Containers/FlatMultiSet.hpp	\
	src/Containers/FlatSet.hpp	\
	src/Containers/List.hpp	\
	src/Containers/StaticVector.hpp	\
	src/Containers/Vector.hpp

pkginclude_Randomdir = ${pkgincludedir}/Random
pkginclude_Random_HEADERS = \
	src/Random/FastGenerator.hpp	\
	src/Random/IsaacGenerator.hpp

pkginclude_Utilitiesdir = ${pkgincludedir}/Utilities
pkginclude_Utilities_HEADERS = \
	src/Utilities/Argv.hpp	\
	src/Utilities/Thunk.hpp

pkginclude_Streamsdir = ${pkgincludedir}/Streams
pkginclude_Streams_HEADERS = \
	src/Streams/AbstractInputStream.hpp	\
	src/Streams/AbstractOutputStream.hpp	\
	src/Streams/BufferInputStream.hpp	\
	src/Streams/BufferOutputStream.hpp	\
	src/Streams/Crc32OutputStream.hpp	\
	src/Streams/Crc64OutputStream.hpp	\
	src/Streams/FileInputStream.hpp	\
	src/Streams/FileOutputStream.hpp	\
	src/Streams/Fnv1a32OutputStream.hpp	\
	src/Streams/Fnv1a64OutputStream.hpp	\
	src/Streams/InputStreamIterator.hpp	\
	src/Streams/Md5OutputStream.hpp	\
	src/Streams/NullInputStream.hpp	\
	src/Streams/NullOutputStream.hpp	\
	src/Streams/OutputStreamIterator.hpp	\
	src/Streams/Sha1OutputStream.hpp	\
	src/Streams/Sha256OutputStream.hpp	\
	src/Streams/StandardErrorStream.hpp	\
	src/Streams/StandardInputStream.hpp	\
	src/Streams/StandardOutputStream.hpp	\
	src/Streams/ZeroInputStream.hpp	\
	src/Streams/StringInputStream.hpp	\
	src/Streams/StringOutputStream.hpp	\
	src/Streams/RandomInputStream.hpp

pkginclude_StreamFiltersdir = ${pkgincludedir}/StreamFilters
pkginclude_StreamFilters_HEADERS = \
	src/StreamFilters/AbstractInputStreamFilter.hpp	\
	src/StreamFilters/AbstractOutputStreamFilter.hpp	\
	src/StreamFilters/BufferingInputStreamFilter.hpp	\
	src/StreamFilters/BufferingOutputStreamFilter.hpp

mcf_sources = \
	src/Core/_KernelObjectBase.cpp	\
	src/Core/_UniqueNtHandle.cpp	\
	src/Core/DynamicLinkLibrary.cpp	\
	src/Core/Exception.cpp	\
	src/Core/File.cpp	\
	src/Core/Rcnts.cpp	\
	src/Core/StreamBuffer.cpp	\
	src/Core/String.cpp	\
	src/Core/StringView.cpp	\
	src/Core/Uuid.cpp	\
	src/Thread/ConditionVariable.cpp	\
	src/Thread/Event.cpp	\
	src/Thread/KernelEvent.cpp	\
	src/Thread/KernelMutex.cpp	\
	src/Thread/KernelRecursiveMutex.cpp	\
	src/Thread/KernelSemaphore.cpp	\
	src/Thread/ReadersWriterMutex.cpp	\
	src/Thread/RecursiveMutex.cpp	\
	src/Thread/Semaphore.cpp	\
	src/Thread/Thread.cpp	\
	src/Thread/UniqueLockBase.cpp	\
	src/SmartPointers/IntrusivePtr.cpp	\
	src/SmartPointers/PolyIntrusivePtr.cpp	\
	src/Random/FastGenerator.cpp	\
	src/Random/IsaacGenerator.cpp	\
	src/Streams/AbstractInputStream.cpp	\
	src/Streams/AbstractOutputStream.cpp	\
	src/Streams/BufferInputStream.cpp	\
	src/Streams/BufferOutputStream.cpp	\
	src/Streams/Crc32OutputStream.cpp	\
	src/Streams/Crc64OutputStream.cpp	\
	src/Streams/FileInputStream.cpp	\
	src/Streams/FileOutputStream.cpp	\
	src/Streams/Fnv1a32OutputStream.cpp	\
	src/Streams/Fnv1a64OutputStream.cpp	\
	src/Streams/Md5OutputStream.cpp	\
	src/Streams/NullInputStream.cpp	\
	src/Streams/NullOutputStream.cpp	\
	src/Streams/Sha1OutputStream.cpp	\
	src/Streams/Sha256OutputStream.cpp	\
	src/Streams/StandardErrorStream.cpp	\
	src/Streams/StandardInputStream.cpp	\
	src/Streams/StandardOutputStream.cpp	\
	src/Streams/ZeroInputStream.cpp	\
	src/Streams/StringInputStream.cpp	\
	src/Streams/StringOutputStream.cpp	\
	src/Streams/RandomInputStream.cpp	\
	src/StreamFilters/AbstractInputStreamFilter.cpp	\
	src/StreamFilters/AbstractOutputStreamFilter.cpp	\
	src/StreamFilters/BufferingInputStreamFilter.cpp	\
	src/StreamFilters/BufferingOutputStreamFilter.cpp

bin_PROGRAMS = \
	MCF-1.dll

MCF_1_dll_SOURCES = \
	${mcf_sources}

MCF_1_dll_LDFLAGS = \
	-shared -nostdlib	\
	-Wl,-e@__MCFCRT_DllStartup,-subsystem,windows	\
	-Wl,--export-all-symbols,--exclude-symbols,@__MCFCRT_DllStartup,--exclude-libs,ALL	\
	-Wl,--disable-stdcall-fixup,--enable-auto-image-base,--out-implib,libMCF.dll.a

MCF_1_dll_LDADD = \
	-lstdc++ -lgcc -lgcc_s -lmingwex -lMCFCRT-pre-dll -lMCFCRT -lmsvcrt -lkernel32 -lntdll

lib_LIBRARIES = \
	libMCF.dll.a	\
	libMCF.a

libMCF_dll_a_SOURCES =
libMCF.dll.a: MCF-1.dll

libMCF_a_SOURCES = \
	${mcf_sources}
