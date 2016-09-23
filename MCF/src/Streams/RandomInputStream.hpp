// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_RANDOM_INPUT_STREAM_HPP_
#define MCF_STREAMS_RANDOM_INPUT_STREAM_HPP_

#include "../Config.hpp"
#include "AbstractInputStream.hpp"

namespace MCF {

class MCF_HAS_EXPORTED_RTTI RandomInputStream : public AbstractInputStream {
public:
	RandomInputStream() noexcept = default;
	~RandomInputStream() override;

	RandomInputStream(RandomInputStream &&) noexcept = default;
	RandomInputStream &operator=(RandomInputStream &&) noexcept = default;

public:
	int Peek() noexcept override;
	int Get() noexcept override;
	bool Discard() noexcept override;
	std::size_t Peek(void *pData, std::size_t uSize) noexcept override;
	std::size_t Get(void *pData, std::size_t uSize) noexcept override;
	std::size_t Discard(std::size_t uSize) noexcept override;

	void Swap(RandomInputStream &rhs) noexcept {
		using std::swap;
		(void)rhs;
	}

public:
	friend void swap(RandomInputStream &lhs, RandomInputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
