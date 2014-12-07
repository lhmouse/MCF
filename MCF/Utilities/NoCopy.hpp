// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_NO_COPY_HPP_
#define MCF_UTILITIES_NO_COPY_HPP_

namespace MCF {

namespace Impl {
	template<unsigned long MAGIC_T>
	struct NonCopyableBase {
		constexpr NonCopyableBase() noexcept = default;
		~NonCopyableBase() = default;

		NonCopyableBase(const NonCopyableBase &) = delete;
		void operator=(const NonCopyableBase &) = delete;
		NonCopyableBase(NonCopyableBase &&) noexcept = delete;
		void operator=(NonCopyableBase &&) noexcept = delete;
	};
}

}

#define NO_COPY		private ::MCF::Impl::NonCopyableBase<__COUNTER__>

#endif
