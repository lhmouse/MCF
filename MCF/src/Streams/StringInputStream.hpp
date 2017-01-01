// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

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
	explicit StringInputStream(NarrowString vString = NarrowString()) noexcept
		: x_vString(std::move(vString)), x_uOffset(0)
	{
	}
	~StringInputStream() override;

	StringInputStream(StringInputStream &&) noexcept = default;
	StringInputStream &operator=(StringInputStream &&) noexcept = default;

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;
	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	const NarrowString &GetString() const noexcept {
		return x_vString;
	}
	NarrowString &GetString() noexcept {
		return x_vString;
	}
	void SetString(NarrowString vString) noexcept {
		x_vString = std::move(vString);
		x_uOffset = 0;
	}

	std::size_t GetOffset() const noexcept {
		return x_uOffset;
	}
	void SetOffset(std::size_t uOffset) noexcept {
		x_uOffset = uOffset;
	}

	void Swap(StringInputStream &rhs) noexcept {
		using std::swap;
		swap(x_vString, rhs.x_vString);
	}

public:
	friend void swap(StringInputStream &lhs, StringInputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
