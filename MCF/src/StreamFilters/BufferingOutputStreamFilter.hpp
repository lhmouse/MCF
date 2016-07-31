// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BUFFERING_OUTPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_BUFFERING_OUTPUT_STREAM_FILTER_HPP_

#include "AbstractOutputStreamFilter.hpp"

namespace MCF {

class BufferingOutputStreamFilter : public AbstractOutputStreamFilter {
public:
	explicit BufferingOutputStreamFilter(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream) noexcept
		: AbstractOutputStreamFilter(std::move(pUnderlyingStream))
	{
	}
	~BufferingOutputStreamFilter() override;

	BufferingOutputStreamFilter(BufferingOutputStreamFilter &&) noexcept = default;
	BufferingOutputStreamFilter &operator=(BufferingOutputStreamFilter &&) noexcept = default;

public:
	void Put(unsigned char byData) override;
	void Put(const void *pData, std::size_t uSize) override;
	void Flush(bool bHard) override;

	void Swap(BufferingOutputStreamFilter &rhs) noexcept {
		using std::swap;
		swap(y_vStream, rhs.y_vStream);
	}

public:
	friend void swap(BufferingOutputStreamFilter &lhs, BufferingOutputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
