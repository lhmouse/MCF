// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_NONCOPYABLE_HPP_
#define MCF_UTILITIES_NONCOPYABLE_HPP_

namespace MCF {

namespace Impl {
	template<unsigned long MAGIC_T>
	struct Noncopyable {
		constexpr Noncopyable() noexcept = default;
		~Noncopyable() = default;

		Noncopyable(const Noncopyable &) = delete;
		void operator=(const Noncopyable &) = delete;
		Noncopyable(Noncopyable &&) noexcept = delete;
		void operator=(Noncopyable &&) noexcept = delete;
	};
}

}

#define Noncopyable		private ::MCF::Impl::Noncopyable<__COUNTER__>

#endif
