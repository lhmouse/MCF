// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_NONCOPYABLE_HPP_
#define MCF_UTILITIES_NONCOPYABLE_HPP_

namespace MCF {

namespace Impl {
	template<unsigned long long MAGIC_T>
	struct NoncopyableBase {
		constexpr NoncopyableBase() noexcept = default;

		NoncopyableBase(const NoncopyableBase &) = delete;
		NoncopyableBase(NoncopyableBase &&) noexcept = delete;
		NoncopyableBase &operator=(const NoncopyableBase &) = delete;
		NoncopyableBase &operator=(NoncopyableBase &&) noexcept = delete;
	};
}

}

#define NONCOPYABLE		private ::MCF::Impl::NoncopyableBase<__COUNTER__>

#endif
