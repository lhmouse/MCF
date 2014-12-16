// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MONITOR_HPP_
#define MCF_THREAD_MONITOR_HPP_

#include "UserRecursiveMutex.hpp"
#include "../Utilities/Noncopyable.hpp"
#include <utility>

namespace MCF {

template<class ObjectT>
class Monitor;

template<class ObjectT>
class MonitorLock {
	friend Monitor<const volatile ObjectT>;
	friend Monitor<const ObjectT>;
	friend Monitor<volatile ObjectT>;
	friend Monitor<ObjectT>;

private:
	UserRecursiveMutex::UniqueLock xm_vLock;
	ObjectT &xm_vObject;

private:
	MonitorLock(UserRecursiveMutex &vMutex, ObjectT &vObject) noexcept
		: xm_vLock(vMutex), xm_vObject(vObject)
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

template<class ObjectT>
class Monitor : NONCOPYABLE {
private:
	mutable UserRecursiveMutex xm_vMutex;
	ObjectT xm_vObject;

public:
	template<typename ...ParamsT>
	explicit Monitor(ParamsT &&...vParams)
		: xm_vObject(std::forward<ParamsT>(vParams)...)
	{
	}

public:
	MonitorLock<const ObjectT> operator*() const noexcept {
		return MonitorLock<const ObjectT>(xm_vMutex, xm_vObject);
	}
	MonitorLock<ObjectT> operator*() noexcept {
		return MonitorLock<ObjectT>(xm_vMutex, xm_vObject);
	}

	MonitorLock<const ObjectT> operator->() const noexcept {
		return MonitorLock<const ObjectT>(xm_vMutex, xm_vObject);
	}
	MonitorLock<ObjectT> operator->() noexcept {
		return MonitorLock<ObjectT>(xm_vMutex, xm_vObject);
	}
};

}

#endif
