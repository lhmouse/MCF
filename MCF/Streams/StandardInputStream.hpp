// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STANDARD_INPUT_STREAM_HPP_
#define MCF_STREAMS_STANDARD_INPUT_STREAM_HPP_

#include "AbstractInputStream.hpp"

namespace MCF {

class StandardInputStream : public AbstractInputStream {
public:
	StandardInputStream() noexcept = default;
	~StandardInputStream() override;

	StandardInputStream(StandardInputStream &&) noexcept = default;
	StandardInputStream &operator=(StandardInputStream &&) noexcept = default;

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;

	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	bool IsEchoing() const noexcept;
	void SetEchoing(bool bEchoing);

	void Swap(StandardInputStream &rhs) noexcept {
		using std::swap;
		(void)rhs;
	}

public:
	friend void swap(StandardInputStream &lhs, StandardInputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
