// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ABSTRACT_OUTPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_ABSTRACT_OUTPUT_STREAM_FILTER_HPP_

#include "../Config.hpp"
#include "../Streams/AbstractOutputStream.hpp"
#include "_BufferedOutputStream.hpp"

namespace MCF {

class MCF_HAS_EXPORTED_RTTI AbstractOutputStreamFilter : public AbstractOutputStream {
protected:
	Impl_BufferedOutputStream::BufferedOutputStream y_vStream;

public:
	explicit AbstractOutputStreamFilter(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream) noexcept
		: y_vStream(std::move(pUnderlyingStream))
	{
	}
	virtual ~AbstractOutputStreamFilter() = 0;

	AbstractOutputStreamFilter(AbstractOutputStreamFilter &&) noexcept = default;
	AbstractOutputStreamFilter &operator=(AbstractOutputStreamFilter &&) noexcept = default;

protected:
	void Y_Swap(AbstractOutputStreamFilter &rhs) noexcept {
		using std::swap;
		swap(y_vStream, rhs.y_vStream);
	}

public:
	virtual void Put(unsigned char byData) = 0;
	virtual void Put(const void *pData, std::size_t uSize) = 0;
	virtual void Flush(bool bHard) = 0;

	const PolyIntrusivePtr<AbstractOutputStream> &GetUnderlyingStream() const noexcept {
		return y_vStream.GetUnderlyingStream();
	}
};

}

#endif
