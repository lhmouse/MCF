// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ABSTRACT_INPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_ABSTRACT_INPUT_STREAM_FILTER_HPP_

#include "../Streams/AbstractInputStream.hpp"

namespace MCF {

class AbstractInputStreamFilter : public AbstractInputStream {
private:
	PolyIntrusivePtr<AbstractInputStream> x_pUnderlyingStream;

public:
	explicit AbstractInputStreamFilter(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
		: x_pUnderlyingStream(std::move(pUnderlyingStream))
	{ }
	~AbstractInputStreamFilter() override;

public:
	int Peek() override = 0;
	int Get() override = 0;
	bool Discard() override = 0;
	std::size_t Peek(void *pData, std::size_t uSize) override = 0;
	std::size_t Get(void *pData, std::size_t uSize) override = 0;
	std::size_t Discard(std::size_t uSize) override = 0;
	void Invalidate() override = 0;

	const PolyIntrusivePtr<AbstractInputStream> &GetUnderlyingStream() const noexcept {
		return x_pUnderlyingStream;
	}
	void SetUnderlyingStream(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream){
		Invalidate();
		x_pUnderlyingStream = std::move(pUnderlyingStream);
	}
};

}

#endif
