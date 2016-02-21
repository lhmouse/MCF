// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_BUFFERING_INPUT_STREAM_FILTER_HPP_
#define MCF_STREAMS_BUFFERING_INPUT_STREAM_FILTER_HPP_

#include "AbstractInputStream.hpp"
#include "../SmartPointers/UniquePtr.hpp"
#include "../Core/StreamBuffer.hpp"

namespace MCF {

class BufferingInputStreamFilter : public AbstractInputStream {
private:
	UniquePtr<AbstractInputStream> x_pUnderlyingStream;
	mutable StreamBuffer x_vBuffer;

public:
	explicit BufferingInputStreamFilter(UniquePtr<AbstractInputStream> pUnderlyingStream) noexcept
		: x_pUnderlyingStream(std::move(pUnderlyingStream))
	{
	}
	~BufferingInputStreamFilter() override;

	BufferingInputStreamFilter(BufferingInputStreamFilter &&) noexcept = default;
	BufferingInputStreamFilter& operator=(BufferingInputStreamFilter &&) noexcept = default;

public:
	int Peek() const override;
	int Get() override;
	bool Discard() override;

	std::size_t Peek(void *pData, std::size_t uSize) const override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	const UniquePtr<AbstractInputStream> &GetUnderlyingStream() const noexcept {
		return x_pUnderlyingStream;
	}
	UniquePtr<AbstractInputStream> &GetUnderlyingStream() noexcept {
		return x_pUnderlyingStream;
	}
	void SetUnderlyingStream(UniquePtr<AbstractInputStream> pUnderlyingStream) noexcept {
		x_pUnderlyingStream = std::move(pUnderlyingStream);
	}

	void Swap(BufferingInputStreamFilter &rhs) noexcept {
		using std::swap;
		swap(x_pUnderlyingStream, rhs.x_pUnderlyingStream);
		swap(x_vBuffer,           rhs.x_vBuffer);
	}

	friend void swap(BufferingInputStreamFilter &lhs, BufferingInputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
