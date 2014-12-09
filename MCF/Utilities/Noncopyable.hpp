// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_NONCOPYABLE_HPP_
#define MCF_UTILITIES_NONCOPYABLE_HPP_

namespace MCF {

namespace Impl {
	template<unsigned long MAGIC_T>
	struct NONCOPYABLE {
		constexpr NONCOPYABLE() noexcept = default;
		~NONCOPYABLE() = default;

		NONCOPYABLE(const NONCOPYABLE &) = delete;
		void operator=(const NONCOPYABLE &) = delete;
		NONCOPYABLE(NONCOPYABLE &&) noexcept = delete;
		void operator=(NONCOPYABLE &&) noexcept = delete;
	};
}

}

#define NONCOPYABLE		private ::MCF::Impl::NONCOPYABLE<__COUNTER__>

#endif
