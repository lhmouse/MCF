// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BUFFERING_INPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_BUFFERING_INPUT_STREAM_FILTER_HPP_

#include "../Config.hpp"
#include "AbstractInputStreamFilter.hpp"

namespace MCF {

class MCF_HAS_EXPORTED_RTTI BufferingInputStreamFilter : public AbstractInputStreamFilter {
public:
	explicit BufferingInputStreamFilter(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
		: AbstractInputStreamFilter(std::move(pUnderlyingStream))
	{
	}
	~BufferingInputStreamFilter() override;

	BufferingInputStreamFilter(BufferingInputStreamFilter &&) noexcept = default;
	BufferingInputStreamFilter &operator=(BufferingInputStreamFilter &&) noexcept = default;

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;
	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	void Swap(BufferingInputStreamFilter &rhs) noexcept {
		AbstractInputStreamFilter::Y_Swap(rhs);
		using std::swap;
	}

public:
	friend void swap(BufferingInputStreamFilter &lhs, BufferingInputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
