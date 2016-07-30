// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "IntrusivePtr.hpp"
#include "../Core/FixedSizeAllocator.hpp"

namespace MCF {

namespace Impl_IntrusivePtr {
	FixedSizeAllocator<kWeakViewSize> g_vViewAllocator;
}

}
