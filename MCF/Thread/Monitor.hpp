// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MONITOR_HPP_
#define MCF_THREAD_MONITOR_HPP_

#include "../Utilities/Noncopyable.hpp"
#include <utility>

namespace MCF {

class Mutex;
class RecursiveMutex;
class KernelMutex;

template<class ObjectT, class MutexT>
class MonitorTemplate;

template<class ObjectT, class MutexT>
class MonitorLock {
	friend MonitorTemplate<const volatile ObjectT, MutexT>;
	friend MonitorTemplate<const ObjectT, MutexT>;
	friend MonitorTemplate<volatile ObjectT, MutexT>;
	friend MonitorTemplate<ObjectT, MutexT>;

private:
	ObjectT &xm_vObject;
	typename MutexT::UniqueLock xm_vLock;

private:
	MonitorLock(ObjectT &vObject, MutexT &vMutex) noexcept
		: xm_vObject(vObject), xm_vLock(vMutex)
	{
	}

	MonitorLock(MonitorLock &&rhs) noexcept = default;

	MonitorLock(const MonitorLock &) = delete;
	MonitorLock &operator=(const MonitorLock &) = delete;
	MonitorLock &operator=(MonitorLock &&) = delete;

public:
	operator ObjectT &() const && noexcept {
		return xm_vObject;
	}

	decltype(auto) operator->() const && noexcept {
		return &xm_vObject;
	}
};

template<class ObjectT, class MutexT>
class MonitorTemplate : NONCOPYABLE {
private:
	ObjectT xm_vObject;
	mutable MutexT xm_vMutex;

public:
	template<typename ...ParamsT>
	explicit MonitorTemplate(ParamsT &&...vParams)
		: xm_vObject(std::forward<ParamsT>(vParams)...)
	{
	}

public:
	MonitorLock<const ObjectT, MutexT> operator*() const noexcept {
		return MonitorLock<const ObjectT, MutexT>(xm_vObject, xm_vMutex);
	}
	MonitorLock<ObjectT, MutexT> operator*() noexcept {
		return MonitorLock<ObjectT, MutexT>(xm_vObject, xm_vMutex);
	}

	MonitorLock<const ObjectT, MutexT> operator->() const noexcept {
		return MonitorLock<const ObjectT, MutexT>(xm_vObject, xm_vMutex);
	}
	MonitorLock<ObjectT, MutexT> operator->() noexcept {
		return MonitorLock<ObjectT, MutexT>(xm_vObject, xm_vMutex);
	}
};

template<class ObjectT>
using Monitor = MonitorTemplate<ObjectT, Mutex>;

template<class ObjectT>
using RecursiveMonitor = MonitorTemplate<ObjectT, RecursiveMutex>;

template<class ObjectT>
using KernelMonitor = MonitorTemplate<ObjectT, KernelMutex>;

}

#endif
