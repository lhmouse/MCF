// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_MONITOR_PTR_HPP_
#define MCF_MONITOR_PTR_HPP_

#include "CriticalSection.hpp"
#include "../Core/Utilities.hpp"
#include <memory>
#include <cstddef>

namespace MCF {

namespace Impl {
	template<class SmartPtr_t>
	class MonitorPtr : NO_COPY {
	public:
		typedef typename SmartPtr_t::element_type ObjectType;

	private:
		template<class Return_t>
		class xMonitorHolder : NO_COPY {
			friend MonitorPtr;

		private:
			Return_t *const xm_pObject;
			CriticalSection::Lock xm_vLock;

		private:
			explicit xMonitorHolder(MonitorPtr &vMonitorPtr) noexcept
				: xm_pObject(vMonitorPtr.xm_pObject.get())
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
		SmartPtr_t xm_pObject;
		std::unique_ptr<CriticalSection> xm_pcsMutex;

	public:
		explicit MonitorPtr(SmartPtr_t pObject) noexcept
			: xm_pObject(std::move(pObject))
			, xm_pcsMutex(CriticalSection::Create())
		{
		}

	public:
		xMonitorHolder<const ObjectType> operator->() const noexcept {
			return xMonitorHolder<const ObjectType>(*this);
		}
		xMonitorHolder<ObjectType> operator->() noexcept {
			return xMonitorHolder<ObjectType>(*this);
		}
	};
}

template<class Object_t>
using UniqueMonitorPtr = Impl::MonitorPtr<std::unique_ptr<Object_t>>;

template<class Object_t>
using SharedMonitorPtr = Impl::MonitorPtr<std::shared_ptr<Object_t>>;

}

#endif
