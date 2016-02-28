// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_BUFFERING_OUTPUT_STREAM_FILTER_HPP_
#define MCF_STREAMS_BUFFERING_OUTPUT_STREAM_FILTER_HPP_

#include "AbstractOutputStreamFilter.hpp"
#include "../Core/StreamBuffer.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

class BufferingOutputStreamFilter : public AbstractOutputStreamFilter {
private:
	mutable StreamBuffer x_vBuffer;
	mutable Vector<unsigned char> x_vecBackBuffer;

public:
	explicit BufferingOutputStreamFilter(UniquePtr<AbstractOutputStream> pUnderlyingStream) noexcept
		: AbstractOutputStreamFilter(std::move(pUnderlyingStream))
	{
	}
	~BufferingOutputStreamFilter() override;

	BufferingOutputStreamFilter(BufferingOutputStreamFilter &&) noexcept = default;
	BufferingOutputStreamFilter& operator=(BufferingOutputStreamFilter &&) noexcept = default;

public:
	void Put(unsigned char byData) override;

	void Put(const void *pData, std::size_t uSize) override;

	void Flush(bool bHard) override;

	void Swap(BufferingOutputStreamFilter &rhs) noexcept {
		using std::swap;
		swap(x_pUnderlyingStream, rhs.x_pUnderlyingStream);
		swap(x_vBuffer,           rhs.x_vBuffer);
		swap(x_vecBackBuffer,     rhs.x_vecBackBuffer);
	}

	friend void swap(BufferingOutputStreamFilter &lhs, BufferingOutputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
