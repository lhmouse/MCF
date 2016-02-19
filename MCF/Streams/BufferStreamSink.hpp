// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_BUFFER_STREAM_SINK_HPP_
#define MCF_STREAMS_BUFFER_STREAM_SINK_HPP_

#include "AbstractStreamSink.hpp"
#include "../Core/StreamBuffer.hpp"

namespace MCF {

class BufferStreamSink : public AbstractStreamSink {
private:
	StreamBuffer x_vBuffer;

public:
	constexpr BufferStreamSink() noexcept
		: x_vBuffer()
	{
	}
	explicit BufferStreamSink(StreamBuffer &sbufBuffer) noexcept
		: BufferStreamSink()
	{
		SetBuffer(std::move(sbufBuffer));
	}
	~BufferStreamSink() override;

public:
	void Put(unsigned char byData) override {
		x_vBuffer.Put(byData);
	}

	void Put(const void *pData, std::size_t uSize) override {
		x_vBuffer.Put(pData, uSize);
	}
	void Put(unsigned char byData, std::size_t uSize) override {
		x_vBuffer.Put(byData, uSize);
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
