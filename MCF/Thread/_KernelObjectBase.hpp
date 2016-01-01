// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_KERNEL_OBJECT_BASE_HPP_
#define MCF_THREAD_KERNEL_OBJECT_BASE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/_UniqueNtHandle.hpp"
#include <cstdint>

namespace MCF {

namespace Impl_KernelObjectBase {
	class KernelObjectBase : NONCOPYABLE {
	public:
		enum : std::uint32_t {
			kSessionLocal  = 0x00000000, // 选择 \Sessions\<SessionId>\BaseNamedObjects 目录。
			kGlobal        = 0x00000001, // 选择 \BaseNamedObjects 目录。

			kDontCreate    = 0x00000002, // 不存在则失败。若指定该选项则 kFailIfExists 无效。
			kFailIfExists  = 0x00000004, // 已存在则失败。
		};

	protected:
		static Impl_UniqueNtHandle::UniqueNtHandle X_OpenBaseNamedObjectDirectory(std::uint32_t u32Flags);
	};
}

}

#endif
