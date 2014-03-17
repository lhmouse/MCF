// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_MUTEX_HPP__
#define __MCF_MUTEX_HPP__

#include "NoCopy.hpp"
#include "UniqueHandle.hpp"
#include <memory>
#include <utility>

namespace MCF {

class Mutex : NO_COPY {
private:
	class xDelegate;

	struct xUnlocker {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *pDelegate) const noexcept {
			xUnlock((xDelegate *)pDelegate);
		}
	};

private:
	static void xUnlock(xDelegate *pDelegate) noexcept;

public:
	typedef UniqueHandle<xUnlocker> LockHolder;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit Mutex(const wchar_t *pwszName = nullptr);
	Mutex(Mutex &&rhs) noexcept;
	Mutex &operator=(Mutex &&rhs) noexcept;
	~Mutex();

public:
	LockHolder Try() noexcept;
	LockHolder Lock() noexcept;
};

}

#define MUTEX_SCOPE(mtx)	\
	for(auto __MCF_LOCK__ = std::make_pair(static_cast<::MCF::Mutex &>(mtx).Lock(), true);	\
		__MCF_LOCK__.second;	\
		__MCF_LOCK__.second = false)

#endif
