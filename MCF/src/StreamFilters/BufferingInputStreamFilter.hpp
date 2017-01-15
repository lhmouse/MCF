// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BUFFERING_INPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_BUFFERING_INPUT_STREAM_FILTER_HPP_

#include "AbstractInputStreamFilter.hpp"
#include "../SmartPointers/UniquePtr.hpp"

namespace MCF {

class BufferingInputStreamFilter : public AbstractInputStreamFilter {
private:
	UniquePtr<unsigned char []> x_pbyBuffer;
	std::size_t x_uCapacity = 0;
	std::size_t x_uOffset = 0;
	std::size_t x_uSize = 0;

public:
	explicit BufferingInputStreamFilter(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
		: AbstractInputStreamFilter(std::move(pUnderlyingStream))
	{
	}
	~BufferingInputStreamFilter() override;

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;
	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;
	void Invalidate() override;
};

}

#endif
