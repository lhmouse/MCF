// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_TEXT_INPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_TEXT_INPUT_STREAM_FILTER_HPP_

#include "AbstractInputStreamFilter.hpp"

namespace MCF {

class TextInputStreamFilter : public AbstractInputStreamFilter {
private:
	StreamBuffer x_sbufPlain;

public:
	explicit TextInputStreamFilter(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
		: AbstractInputStreamFilter(std::move(pUnderlyingStream))
	{
	}
	~TextInputStreamFilter() override;

	TextInputStreamFilter(TextInputStreamFilter &&) noexcept = default;
	TextInputStreamFilter &operator=(TextInputStreamFilter &&) noexcept = default;

private:
	void X_PopulatePlainBuffer(std::size_t uExpected);

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;
	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	void Swap(TextInputStreamFilter &rhs) noexcept {
		AbstractInputStreamFilter::Y_Swap(rhs);
		using std::swap;
		swap(x_sbufPlain, rhs.x_sbufPlain);
	}

public:
	friend void swap(TextInputStreamFilter &lhs, TextInputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
