// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MONITOR_HPP_
#define MCF_THREAD_MONITOR_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/AddressOf.hpp"
#include <utility>

namespace MCF {

template<class ObjectT, class MutexT>
class MonitorTemplate;

namespace Impl_Monitor {
	template<class ObjectT>
	class MonitorViewAsReference {
	private:
		ObjectT &x_vObject;

	public:
		explicit MonitorViewAsReference(ObjectT &vObject) noexcept
			: x_vObject(vObject)
		{
		}

	public:
		decltype(auto) operator&() const && noexcept {
			return &x_vObject;
		}
		operator ObjectT &() const && noexcept {
			return x_vObject;
		}
		template<typename ...ArgsT>
		decltype(auto) operator()(ArgsT &&...vArgs) const && {
			return x_vObject(std::forward<ArgsT>(vArgs)...);
		}
	};
	template<class ObjectT>
	class MonitorViewAsPointer {
	private:
		ObjectT &x_vObject;

	public:
		explicit MonitorViewAsPointer(ObjectT &vObject) noexcept
			: x_vObject(vObject)
		{
		}

	public:
		ObjectT &operator*() const && noexcept {
			return x_vObject;
		}
		ObjectT *operator->() const && noexcept {
			return AddressOf(x_vObject);
		}
	};

	template<class ObjectT, class MutexT, class ViewT>
	class MonitorLock : private MutexT::UniqueLock, public ViewT {
		friend MonitorTemplate<const volatile ObjectT, MutexT>;
		friend MonitorTemplate<const ObjectT, MutexT>;
		friend MonitorTemplate<volatile ObjectT, MutexT>;
		friend MonitorTemplate<ObjectT, MutexT>;

	private:
		MonitorLock(MutexT &vMutex, ObjectT &vObject) noexcept
			: MutexT::UniqueLock(vMutex), ViewT(vObject)
		{
		}

		MonitorLock(const MonitorLock &) = delete;
		MonitorLock(MonitorLock &&rhs) noexcept = default;
	};
}

template<class ObjectT, class MutexT>
class MonitorTemplate : MCF_NONCOPYABLE {
public:
	template<class ViewObjectT>
	using LockedReference = Impl_Monitor::MonitorLock<ViewObjectT, MutexT, Impl_Monitor::MonitorViewAsReference<ViewObjectT>>;

	template<class ViewObjectT>
	using LockedPointer   = Impl_Monitor::MonitorLock<ViewObjectT, MutexT, Impl_Monitor::MonitorViewAsPointer<ViewObjectT>>;

private:
	mutable MutexT x_vMutex;
	ObjectT x_vObject;

public:
	template<typename ...ParamsT>
	explicit MonitorTemplate(ParamsT &&...vParams)
		: x_vObject(std::forward<ParamsT>(vParams)...)
	{
	}

public:
	LockedReference<const ObjectT> operator*() const & noexcept {
		return LockedReference<const ObjectT>(x_vMutex.GetLock(), x_vObject);
	}
	LockedReference<ObjectT> operator*() & noexcept {
		return LockedReference<ObjectT>(x_vMutex.GetLock(), x_vObject);
	}

	LockedPointer<const ObjectT> operator->() const & noexcept {
		return LockedPointer<const ObjectT>(x_vMutex.GetLock(), x_vObject);
	}
	LockedPointer<ObjectT> operator->() & noexcept {
		return LockedPointer<ObjectT>(x_vMutex.GetLock(), x_vObject);
	}
};

template<class ObjectT>
using Monitor                = MonitorTemplate<ObjectT, class Mutex>;
template<class ObjectT>
using RecursiveMonitor       = MonitorTemplate<ObjectT, class RecursiveMutex>;
template<class ObjectT>
using KernelMonitor          = MonitorTemplate<ObjectT, class KernelMutex>;
template<class ObjectT>
using KernelRecursiveMonitor = MonitorTemplate<ObjectT, class KernelRecursiveMutex>;

}

#endif
