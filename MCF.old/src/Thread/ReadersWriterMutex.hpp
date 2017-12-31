// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_READER_WRITER_MUTEX_HPP_
#define MCF_THREAD_READER_WRITER_MUTEX_HPP_

#include "Mutex.hpp"
#include "../Core/Atomic.hpp"

namespace MCF {

class ReadersWriterMutex {
public:
	enum : std::size_t { kSuggestedSpinCount = Mutex::kSuggestedSpinCount };

	struct MutexTraitsAsReader {
		static bool Try(ReadersWriterMutex *pMutex, std::uint64_t u64UntilFastMonoClock){
			return pMutex->TryAsReader(u64UntilFastMonoClock);
		}
		static void Lock(ReadersWriterMutex *pMutex){
			pMutex->LockAsReader();
		}
		static void Unlock(ReadersWriterMutex *pMutex){
			pMutex->UnlockAsReader();
		}
	};
	struct MutexTraitsAsWriter {
		static bool Try(ReadersWriterMutex *pMutex, std::uint64_t u64UntilFastMonoClock){
			return pMutex->TryAsWriter(u64UntilFastMonoClock);
		}
		static void Lock(ReadersWriterMutex *pMutex){
			pMutex->LockAsWriter();
		}
		static void Unlock(ReadersWriterMutex *pMutex){
			pMutex->UnlockAsWriter();
		}
	};

private:
	Mutex x_mtxReaderGuard;
	Mutex x_mtxExclusive;
	Atomic<std::size_t> x_uReaderCount;

public:
	explicit constexpr ReadersWriterMutex(std::size_t uSpinCount = kSuggestedSpinCount) noexcept
		: x_mtxReaderGuard(kSuggestedSpinCount), x_mtxExclusive(uSpinCount), x_uReaderCount(0)
	{ }

	ReadersWriterMutex(const ReadersWriterMutex &) = delete;
	ReadersWriterMutex &operator=(const ReadersWriterMutex &) = delete;

public:
	std::size_t GetSpinCount() const noexcept {
		return x_mtxExclusive.GetSpinCount();
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_mtxExclusive.SetSpinCount(uSpinCount);
	}

	bool TryAsReader(std::uint64_t u64UntilFastMonoClock = 0) noexcept;
	void LockAsReader() noexcept;
	void UnlockAsReader() noexcept;

	UniqueLock<ReadersWriterMutex, MutexTraitsAsReader> TryGetLockAsReader(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		return UniqueLock<ReadersWriterMutex, MutexTraitsAsReader>(*this, u64UntilFastMonoClock);
	}
	UniqueLock<ReadersWriterMutex, MutexTraitsAsReader> GetLockAsReader() noexcept {
		return UniqueLock<ReadersWriterMutex, MutexTraitsAsReader>(*this);
	}

	bool TryAsWriter(std::uint64_t u64UntilFastMonoClock = 0) noexcept;
	void LockAsWriter() noexcept;
	void UnlockAsWriter() noexcept;

	UniqueLock<ReadersWriterMutex, MutexTraitsAsWriter> TryGetLockAsWriter(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		return UniqueLock<ReadersWriterMutex, MutexTraitsAsWriter>(*this, u64UntilFastMonoClock);
	}
	UniqueLock<ReadersWriterMutex, MutexTraitsAsWriter> GetLockAsWriter() noexcept {
		return UniqueLock<ReadersWriterMutex, MutexTraitsAsWriter>(*this);
	}
};

static_assert(std::is_trivially_destructible<ReadersWriterMutex>::value, "Hey!");

}

#endif
