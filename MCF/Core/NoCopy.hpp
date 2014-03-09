// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_NO_COPY_HPP__
#define __MCF_NO_COPY_HPP__

namespace MCF {

namespace __MCF {
	struct NonCopyable {
		constexpr NonCopyable() noexcept = default;
		~NonCopyable() = default;

		NonCopyable(const NonCopyable &) = delete;
		void operator=(const NonCopyable &) = delete;
		NonCopyable(NonCopyable &&) = delete;
		void operator=(NonCopyable &&) = delete;
	};
}

}

#define NO_COPY		private ::MCF::__MCF::NonCopyable

#endif
