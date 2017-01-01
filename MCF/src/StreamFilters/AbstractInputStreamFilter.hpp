// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ABSTRACT_INPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_ABSTRACT_INPUT_STREAM_FILTER_HPP_

#include "../Streams/AbstractInputStream.hpp"
#include "_BufferedInputStream.hpp"

namespace MCF {

class AbstractInputStreamFilter : public AbstractInputStream {
protected:
	Impl_BufferedInputStream::BufferedInputStream y_vStream;

public:
	explicit AbstractInputStreamFilter(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
		: y_vStream(std::move(pUnderlyingStream))
	{
	}
	~AbstractInputStreamFilter() override = 0;

	AbstractInputStreamFilter(AbstractInputStreamFilter &&) noexcept = default;
	AbstractInputStreamFilter &operator=(AbstractInputStreamFilter &&) noexcept = default;

protected:
	void Y_Swap(AbstractInputStreamFilter &rhs) noexcept {
		using std::swap;
		swap(y_vStream, rhs.y_vStream);
	}

public:
	virtual int Peek() override = 0;
	virtual int Get() override = 0;
	virtual bool Discard() override = 0;
	virtual std::size_t Peek(void *pData, std::size_t uSize) override = 0;
	virtual std::size_t Get(void *pData, std::size_t uSize) override = 0;
	virtual std::size_t Discard(std::size_t uSize) override = 0;

	const PolyIntrusivePtr<AbstractInputStream> &GetUnderlyingStream() const noexcept {
		return y_vStream.GetUnderlyingStream();
	}
};

}

#endif
