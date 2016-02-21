// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_BUFFER_INPUT_STREAM_HPP_
#define MCF_STREAMS_BUFFER_INPUT_STREAM_HPP_

#include "AbstractInputStream.hpp"
#include "../Core/StreamBuffer.hpp"

namespace MCF {

class BufferInputStream : public AbstractInputStream {
private:
	StreamBuffer x_vBuffer;

public:
	constexpr BufferInputStream() noexcept
		: x_vBuffer()
	{
	}
	explicit BufferInputStream(StreamBuffer vBuffer) noexcept
		: x_vBuffer(std::move(vBuffer))
	{
	}
	~BufferInputStream() override;

	BufferInputStream(BufferInputStream &&) noexcept = default;
	BufferInputStream& operator=(BufferInputStream &&) noexcept = default;

public:
	int Peek() const override;
	int Get() override;
	bool Discard() override;

	std::size_t Peek(void *pData, std::size_t uSize) const override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	const StreamBuffer &GetBuffer() const noexcept {
		return x_vBuffer;
	}
	StreamBuffer &GetBuffer() noexcept {
		return x_vBuffer;
	}
	void SetBuffer(StreamBuffer vBuffer) noexcept {
		x_vBuffer = std::move(vBuffer);
	}

	void Swap(BufferInputStream &rhs) noexcept {
		using std::swap;
		swap(x_vBuffer, rhs.x_vBuffer);
	}

	friend void swap(BufferInputStream &lhs, BufferInputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
