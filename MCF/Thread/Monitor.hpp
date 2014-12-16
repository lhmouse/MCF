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

template<class ObjectT>
class MonitorObserverAsReference {
private:
	ObjectT &xm_vObject;

public:
	explicit MonitorObserverAsReference(ObjectT &vObject) noexcept
		: xm_vObject(vObject)
	{
	}

public:
	operator ObjectT &() const && noexcept {
		return xm_vObject;
	}
};
template<class ObjectT>
class MonitorObserverAsPointer {
private:
	ObjectT &xm_vObject;

public:
	explicit MonitorObserverAsPointer(ObjectT &vObject) noexcept
		: xm_vObject(vObject)
	{
	}

public:
	ObjectT &operator*() const && noexcept {
		return xm_vObject;
	}
	decltype(auto) operator->() const && noexcept {
		return &xm_vObject;
	}
};

template<class ObjectT, class MutexT, class ObserverT>
class MonitorLock : public ObserverT {
	friend MonitorTemplate<const volatile ObjectT, MutexT>;
	friend MonitorTemplate<const ObjectT, MutexT>;
	friend MonitorTemplate<volatile ObjectT, MutexT>;
	friend MonitorTemplate<ObjectT, MutexT>;

private:
	typename MutexT::UniqueLock xm_vLock;

private:
	MonitorLock(ObjectT &vObject, MutexT &vMutex) noexcept
		: ObserverT(vObject), xm_vLock(vMutex)
	{
	}

	MonitorLock(MonitorLock &&rhs) noexcept = default;

	MonitorLock(const MonitorLock &) = delete;
	MonitorLock &operator=(const MonitorLock &) = delete;
	MonitorLock &operator=(MonitorLock &&) = delete;
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
	auto operator*() const noexcept {
		return MonitorLock<const ObjectT, MutexT, MonitorObserverAsReference<const ObjectT>>(xm_vObject, xm_vMutex);
	}
	auto operator*() noexcept {
		return MonitorLock<ObjectT, MutexT, MonitorObserverAsReference<ObjectT>>(xm_vObject, xm_vMutex);
	}

	auto operator->() const noexcept {
		return MonitorLock<const ObjectT, MutexT, MonitorObserverAsPointer<const ObjectT>>(xm_vObject, xm_vMutex);
	}
	auto operator->() noexcept {
		return MonitorLock<ObjectT, MutexT, MonitorObserverAsPointer<ObjectT>>(xm_vObject, xm_vMutex);
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
