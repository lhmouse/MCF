// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_KERNEL_OBJECT_BASE_HPP_
#define MCF_CORE_KERNEL_OBJECT_BASE_HPP_

#include "_UniqueNtHandle.hpp"
#include <cstdint>

namespace MCF {

namespace Impl_KernelObjectBase {
	class KernelObjectBase {
	public:
		enum : std::uint32_t {
			kGlobal        = 0x00000001, // 指定 Y_OpenBaseNamedObjectDirectory() 选择 \BaseNamedObjects 目录，否则选择 \Sessions\<SessionId>\BaseNamedObjects 目录。
			kDontCreate    = 0x00000002, // 不存在则失败。
			kFailIfExists  = 0x00000004, // 已存在则失败。
		};

		using Handle = Impl_UniqueNtHandle::Handle;

	protected:
		static Impl_UniqueNtHandle::UniqueNtHandle Y_OpenBaseNamedObjectDirectory(std::uint32_t u32Flags);
	};
}

}

#endif
