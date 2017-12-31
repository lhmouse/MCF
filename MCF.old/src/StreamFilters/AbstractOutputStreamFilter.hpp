// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ABSTRACT_OUTPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_ABSTRACT_OUTPUT_STREAM_FILTER_HPP_

#include "../Streams/AbstractOutputStream.hpp"

namespace MCF {

class AbstractOutputStreamFilter : public AbstractOutputStream {
private:
	PolyIntrusivePtr<AbstractOutputStream> x_pUnderlyingStream;

public:
	explicit AbstractOutputStreamFilter(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream) noexcept
		: x_pUnderlyingStream(std::move(pUnderlyingStream))
	{ }
	~AbstractOutputStreamFilter() override;

public:
	void Put(unsigned char byData) override = 0;
	void Put(const void *pData, std::size_t uSize) override = 0;
	void Flush(bool bHard) override = 0;

	const PolyIntrusivePtr<AbstractOutputStream> &GetUnderlyingStream() const noexcept {
		return x_pUnderlyingStream;
	}
	void SetUnderlyingStream(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream){
		Flush(false);
		x_pUnderlyingStream = std::move(pUnderlyingStream);
	}
};

}

#endif
