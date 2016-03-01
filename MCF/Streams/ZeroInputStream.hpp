// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_ZERO_INPUT_STREAM_HPP_
#define MCF_STREAMS_ZERO_INPUT_STREAM_HPP_

#include "AbstractInputStream.hpp"

namespace MCF {

class ZeroInputStream : public AbstractInputStream {
public:
	ZeroInputStream() noexcept = default;
	~ZeroInputStream() override;

	ZeroInputStream(ZeroInputStream &&) noexcept = default;
	ZeroInputStream& operator=(ZeroInputStream &&) noexcept = default;

public:
	int Peek() const noexcept override;
	int Get() noexcept override;
	bool Discard() noexcept override;

	std::size_t Peek(void *pData, std::size_t uSize) const noexcept override;
	std::size_t Get(void *pData, std::size_t uSize) noexcept override;
	std::size_t Discard(std::size_t uSize) noexcept override;

	void Swap(ZeroInputStream & /* rhs */) noexcept {
		using std::swap;
	}

	friend void swap(ZeroInputStream &lhs, ZeroInputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
