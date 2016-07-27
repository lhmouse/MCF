// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_NONCOPYABLE_HPP_
#define MCF_CORE_NONCOPYABLE_HPP_

namespace MCF {

namespace Impl_Noncopyable {
	template<unsigned long kMagicT>
	struct NoncopyableBase {
		constexpr NoncopyableBase() noexcept = default;

		NoncopyableBase(const NoncopyableBase &) noexcept = delete;
		NoncopyableBase &operator=(const NoncopyableBase &) noexcept = delete;
	};
}

}

#define MCF_NONCOPYABLE     private ::MCF::Impl_Noncopyable::NoncopyableBase<__COUNTER__>

#endif
