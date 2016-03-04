// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ABSTRACT_OUTPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_ABSTRACT_OUTPUT_STREAM_FILTER_HPP_

#include "../Streams/AbstractOutputStream.hpp"

namespace MCF {

class AbstractOutputStreamFilter : public AbstractOutputStream {
protected:
	PolyIntrusivePtr<AbstractOutputStream> x_pUnderlyingStream;

public:
	explicit AbstractOutputStreamFilter(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream) noexcept
		: x_pUnderlyingStream(std::move(pUnderlyingStream))
	{
	}
	virtual ~AbstractOutputStreamFilter() = 0;

	AbstractOutputStreamFilter(AbstractOutputStreamFilter &&) noexcept = default;
	AbstractOutputStreamFilter& operator=(AbstractOutputStreamFilter &&) noexcept = default;

public:
	virtual void Put(unsigned char byData) = 0;

	virtual void Put(const void *pData, std::size_t uSize) = 0;

	virtual void Flush(bool bHard) = 0;

	const PolyIntrusivePtr<AbstractOutputStream> &GetUnderlyingStream() const noexcept {
		return x_pUnderlyingStream;
	}
	void SetUnderlyingStream(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream) noexcept {
		x_pUnderlyingStream = std::move(pUnderlyingStream);
	}
};

}

#endif
