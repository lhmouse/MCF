// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_MONITOR_PTR_HPP_
#define MCF_MONITOR_PTR_HPP_

#include "CriticalSection.hpp"
#include "../Core/Utilities.hpp"
#include <cstddef>

namespace MCF {

template<class Object_t>
class MonitorPtr : NO_COPY {
private:
	template<class Return_t>
	class xMonitorHolder : NO_COPY {
		friend MonitorPtr;

	private:
		Return_t *const xm_pObject;
		CriticalSection::Lock xm_vLock;

	private:
		explicit xMonitorHolder(MonitorPtr &vMonitorPtr) noexcept
			: xm_pObject(&(vMonitorPtr.xm_vObject))
			, xm_vLock(vMonitorPtr.xm_pcsMutex->GetLock())
		{
		}
		xMonitorHolder(xMonitorHolder &&rhs) noexcept
			: xm_pObject(rhs.xm_pObject)
			, xm_vLock(std::move(rhs.xm_vLock))
		{
		}

		void operator=(xMonitorHolder &&rhs) = delete;

	public:
		Return_t *operator->() const noexcept {
			return xm_pObject;
		}
	};

private:
	Object_t xm_vObject;
	std::unique_ptr<CriticalSection> xm_pcsMutex;

public:
	template<typename... Params_t>
	explicit MonitorPtr(Params_t &&... vParams)
		: xm_vObject(std::forward<Params_t>(vParams)...)
		, xm_pcsMutex(CriticalSection::Create())
	{
	}

public:
	xMonitorHolder<const Object_t> operator->() const noexcept {
		return xMonitorHolder<const Object_t>(*this);
	}
	xMonitorHolder<Object_t> operator->() noexcept {
		return xMonitorHolder<Object_t>(*this);
	}
};

}

#endif
