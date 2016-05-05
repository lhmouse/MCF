// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_HPP_
#define MCF_THREAD_THREAD_HPP_

#include "../../MCFCRT/env/thread.h"
#include "../Utilities/Assert.hpp"
#include "../SmartPointers/IntrusivePtr.hpp"
#include "../Core/UniqueHandle.hpp"
#include <exception>
#include <type_traits>
#include <cstddef>

namespace MCF {

class Thread {
private:
	struct X_ThreadCloser {
		constexpr ::_MCFCRT_ThreadHandle operator()() const noexcept {
			return nullptr;
		}
		void operator()(::_MCFCRT_ThreadHandle hThread) const noexcept {
			::_MCFCRT_CloseThread(hThread);
		}
	};

	class X_AbstractControlBlock : public IntrusiveBase<X_AbstractControlBlock> {
	private:
		UniqueHandle<X_ThreadCloser> x_hThread;
		std::uintptr_t x_uTid;

	public:
		virtual ~X_AbstractControlBlock();

	private:
		virtual void X_ThreadProc() = 0;

	public:
		void SpawnThread(bool bSuspended);

		::_MCFCRT_ThreadHandle GetHandle() const noexcept {
			return x_hThread.Get();
		}
		std::uintptr_t GetTid() const noexcept {
			return x_uTid;
		}
	};

	template<typename ProcT>
	class X_ControlBlock : public X_AbstractControlBlock {
	private:
		ProcT x_fnProc;

	public:
		explicit X_ControlBlock(ProcT fnProc)
			: x_fnProc(std::move(fnProc))
		{
		}
		~X_ControlBlock() override {
		}

	private:
		void X_ThreadProc() override {
			x_fnProc();
		}
	};

public:
	using Handle = ::_MCFCRT_ThreadHandle;

public:
	static std::uintptr_t GetCurrentId() noexcept {
		return ::_MCFCRT_GetCurrentThreadId();
	}

	static void Sleep(std::uint64_t u64UntilFastMonoClock) noexcept {
		::_MCFCRT_Sleep(u64UntilFastMonoClock);
	}
	static bool AlertableSleep(std::uint64_t u64UntilFastMonoClock) noexcept {
		return ::_MCFCRT_AlertableSleep(u64UntilFastMonoClock);
	}
	static void AlertableSleep() noexcept {
		::_MCFCRT_AlertableSleepForever();
	}
	static void YieldExecution() noexcept {
		::_MCFCRT_YieldThread();
	}

private:
	IntrusivePtr<X_AbstractControlBlock> x_pControlBlock;

public:
	constexpr Thread() noexcept
		: x_pControlBlock(nullptr)
	{
	}
	template<typename ProcT>
	Thread(ProcT &&fnProc, bool bSuspended)
		: Thread()
	{
		auto pControlBlock = MakeIntrusive<X_ControlBlock<std::decay_t<ProcT>>>(std::forward<ProcT>(fnProc));
		pControlBlock->SpawnThread(bSuspended);
		x_pControlBlock = std::move(pControlBlock);
	}
	Thread(Thread &&rhs) noexcept
		: Thread()
	{
		rhs.Swap(*this);
	}
	Thread &operator=(Thread &&rhs) noexcept {
		Thread(std::move(rhs)).Swap(*this);
		return *this;
	}
	~Thread(void){
		if(!IsNull()){
			std::terminate();
		}
	}

public:
	bool IsNull() const noexcept {
		return !x_pControlBlock;
	}
	Handle GetHandle() const noexcept {
		return x_pControlBlock ? x_pControlBlock->GetHandle() : nullptr;
	}
	std::uintptr_t GetTid() const noexcept {
		return x_pControlBlock ? x_pControlBlock->GetTid() : 0;
	}

	template<typename ProcT>
	void Create(ProcT &&fnProc, bool bSuspended){
		Thread(std::forward<ProcT>(fnProc), bSuspended).Swap(*this);
	}
	void Join() noexcept {
		if(x_pControlBlock){
			Wait();
			x_pControlBlock = nullptr;
		}
	}

	bool Wait(std::uint64_t u64UntilFastMonoClock) const noexcept {
		MCF_ASSERT(x_pControlBlock);

		return ::_MCFCRT_WaitForThread(x_pControlBlock->GetHandle(), u64UntilFastMonoClock);
	}
	void Wait() const noexcept {
		MCF_ASSERT(x_pControlBlock);

		::_MCFCRT_WaitForThreadForever(x_pControlBlock->GetHandle());
	}

	void Suspend() noexcept {
		MCF_ASSERT(x_pControlBlock);

		::_MCFCRT_SuspendThread(x_pControlBlock->GetHandle());
	}
	void Resume() noexcept {
		MCF_ASSERT(x_pControlBlock);

		::_MCFCRT_ResumeThread(x_pControlBlock->GetHandle());
	}

	void Swap(Thread &rhs) noexcept {
		using std::swap;
		swap(x_pControlBlock, rhs.x_pControlBlock);
	}

public:
	explicit operator bool() const noexcept {
		return !IsNull();
	}

	friend void swap(Thread &lhs, Thread &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
