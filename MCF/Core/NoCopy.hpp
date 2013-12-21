// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_NOCOPY_HPP__
#define __MCF_NOCOPY_HPP__

namespace MCF {
	namespace __MCF {
		struct NonCopyable {
			NonCopyable(const NonCopyable &) = delete;
			void operator=(const NonCopyable &) = delete;
		};
	}
}

#define NOCOPY	private ::MCF::__MCF::NonCopyable

#endif
