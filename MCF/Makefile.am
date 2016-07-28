ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = no-exeext foreign subdir-objects
AM_CPPFLAGS = -Wall -Wextra -pedantic -pedantic-errors -Werror -Wno-error=unused-parameter	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel
AM_CFLAGS = -std=c11 -Wstrict-prototypes
AM_CXXFLAGS = -std=c++14 -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -fnothrow-opt

## I think you GNU people should just STFU and stop confusing the linker.
EXEEXT =

pkginclude_Coredir = ${pkgincludedir}/Core
pkginclude_Core_HEADERS =	\
	src/Core/_CheckedSizeArithmetic.hpp	\
	src/Core/_Enumerator.hpp	\
	src/Core/_ForwardedParam.hpp	\
	src/Core/_MagicalInvoker.hpp	\
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
	src/Core/BlackHole.hpp	\
	src/Core/Casters.hpp	\
	src/Core/Clocks.hpp	\
	src/Core/Clone.hpp	\
	src/Core/Comparators.hpp	\
	src/Core/ConstructDestruct.hpp	\
	src/Core/CountLeadingTrailingZeroes.hpp	\
	src/Core/CopyMoveFill.hpp	\
	src/Core/CountOf.hpp	\
	src/Core/DeclVal.hpp	\
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
	src/Core/ParameterPackManipulators.hpp	\
	src/Core/Random.hpp	\
	src/Core/Rcnts.hpp	\
	src/Core/ReconstructOrAssign.hpp	\
	src/Core/RefWrapper.hpp	\
	src/Core/String.hpp	\
	src/Core/StringView.hpp	\
	src/Core/TupleManipulators.hpp	\
	src/Core/UniqueHandle.hpp	\
	src/Core/Uuid.hpp	\
	src/Core/WhiteHole.hpp	\
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
	src/Thread/UniqueLockBase.hpp	\
	src/SmartPointers/DefaultDeleter.hpp	\
	src/SmartPointers/IntrusivePtr.hpp	\
	src/SmartPointers/PolyIntrusivePtr.hpp	\
	src/SmartPointers/UniquePtr.hpp	\
	src/Containers/_FlatContainer.hpp	\
	src/Containers/CircularQueue.hpp	\
	src/Containers/FlatMap.hpp	\
	src/Containers/FlatMultiMap.hpp	\
	src/Containers/FlatMultiSet.hpp	\
	src/Containers/FlatSet.hpp	\
	src/Containers/List.hpp	\
	src/Containers/StaticVector.hpp	\
	src/Containers/Vector.hpp	\
	src/Random/FastGenerator.hpp	\
	src/Random/IsaacGenerator.hpp	\
	src/Utilities/Argv.cpp	\
	src/Utilities/Thunk.cpp	\
	src/Utilities/Variant.cpp

mcf_sources =	\
	src/Core/_KernelObjectBase.cpp	\
	src/Core/_UniqueNtHandle.cpp	\
	src/Core/DynamicLinkLibrary.cpp	\
	src/Core/Exception.cpp	\
	src/Core/File.cpp	\
	src/Core/Rcnts.cpp	\
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
	src/Utilities/Argv.cpp	\
	src/Utilities/Thunk.cpp	\
	src/Utilities/Variant.cpp

bin_PROGRAMS =	\
	MCF-1.dll

MCF_1_dll_SOURCES =	\
	${mcf_sources}

MCF_1_dll_LDFLAGS =	\
	-shared -static -nostdlib	\
	-Wl,-e@__MCFCRT_DllStartup,-subsystem,windows,--disable-runtime-pseudo-reloc,--disable-auto-import	\
	-Wl,--export-all-symbols,--exclude-symbols,@__MCFCRT_DllStartup,--exclude-libs,ALL	\
	-Wl,--disable-stdcall-fixup,--enable-auto-image-base,--out-implib,libMCF.dll.a

MCF_1_dll_LDADD =	\
	-lMCFCRT-pre -lsupc++ -lgcc -lgcc_eh -lmingwex -lMCFCRT -lmsvcrt -lkernel32 -lntdll

lib_LIBRARIES =	\
	libMCF.dll.a	\
	libMCF.a

libMCF.dll.a: MCF-1.dll

libMCF_a_SOURCES =	\
	${mcf_sources}
