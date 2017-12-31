// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STRING_INPUT_STREAM_HPP_
#define MCF_STREAMS_STRING_INPUT_STREAM_HPP_

#include "AbstractInputStream.hpp"
#include "../Core/String.hpp"

namespace MCF {

class StringInputStream : public AbstractInputStream {
private:
	NarrowString x_vString;
	std::size_t x_uOffset;

public:
	explicit StringInputStream(NarrowString vString = NarrowString(), std::size_t uOffset = 0) noexcept
		: x_vString(std::move(vString)), x_uOffset(uOffset)
	{ }
	~StringInputStream() override;

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;
	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;
	void Invalidate() override;

	const NarrowString &GetString() const noexcept {
		return x_vString;
	}
	NarrowString &GetString() noexcept {
		return x_vString;
	}
	void SetString(NarrowString &&vString, std::size_t uOffset = 0) noexcept {
		x_vString = std::move(vString);
		x_uOffset = uOffset;
	}

	std::size_t GetOffset() const noexcept {
		return x_uOffset;
	}
	void SetOffset(std::size_t uOffset) noexcept {
		x_uOffset = uOffset;
	}
};

}

#endif
