// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_BUFFER_STREAM_SOURCE_HPP_
#define MCF_STREAMS_BUFFER_STREAM_SOURCE_HPP_

#include "AbstractStreamSource.hpp"
#include "../Core/StreamBuffer.hpp"

namespace MCF {

class BufferStreamSource : public AbstractStreamSource {
private:
	StreamBuffer x_vBuffer;

public:
	constexpr BufferStreamSource() noexcept
		: x_vBuffer()
	{
	}
	explicit BufferStreamSource(StreamBuffer &sbufBuffer) noexcept
		: BufferStreamSource()
	{
		SetBuffer(std::move(sbufBuffer));
	}
	~BufferStreamSource() override;

public:
	int Peek() const noexcept override {
		return x_vBuffer.Peek();
	}
	int Get() noexcept override {
		return x_vBuffer.Get();
	}
	void Discard() noexcept override {
		return x_vBuffer.Discard();
	}

	std::size_t Peek(void *pData, std::size_t uSize) const noexcept override {
		return x_vBuffer.Peek(pData, uSize);
	}
	std::size_t Get(void *pData, std::size_t uSize) noexcept override {
		return x_vBuffer.Get(pData, uSize);
	}
	std::size_t Discard(std::size_t uSize) noexcept override {
		return x_vBuffer.Discard(uSize);
	}

	const StreamBuffer &GetBuffer() const noexcept {
		return x_vBuffer;
	}
	StreamBuffer &GetBuffer() noexcept {
		return x_vBuffer;
	}
	void SetBuffer(StreamBuffer sbufBuffer) noexcept {
		x_vBuffer = std::move(sbufBuffer);
	}
};

}

#endif
