// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BUFFERING_INPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_BUFFERING_INPUT_STREAM_FILTER_HPP_

#include "AbstractInputStreamFilter.hpp"
#include "../Core/StreamBuffer.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

class BufferingInputStreamFilter : public AbstractInputStreamFilter {
private:
	mutable StreamBuffer x_vBuffer;
	mutable Vector<unsigned char> x_vecBackBuffer;

public:
	explicit BufferingInputStreamFilter(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
		: AbstractInputStreamFilter(std::move(pUnderlyingStream))
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

	void Swap(BufferingInputStreamFilter &rhs) noexcept {
		using std::swap;
		swap(x_pUnderlyingStream, rhs.x_pUnderlyingStream);
		swap(x_vBuffer,           rhs.x_vBuffer);
		swap(x_vecBackBuffer,     rhs.x_vecBackBuffer);
	}

	friend void swap(BufferingInputStreamFilter &lhs, BufferingInputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
