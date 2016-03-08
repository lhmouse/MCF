// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BASE64_INPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_BASE64_INPUT_STREAM_FILTER_HPP_

#include "AbstractInputStreamFilter.hpp"

namespace MCF {

class Base64InputStreamFilter : public AbstractInputStreamFilter {
private:
	StreamBuffer x_sbufPlain;

public:
	explicit Base64InputStreamFilter(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
		: AbstractInputStreamFilter(std::move(pUnderlyingStream))
	{
	}
	~Base64InputStreamFilter() override;

	Base64InputStreamFilter(Base64InputStreamFilter &&) noexcept = default;
	Base64InputStreamFilter &operator=(Base64InputStreamFilter &&) noexcept = default;

private:
	void X_PopulatePlainBuffer(std::size_t uExpected);

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;

	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	void Swap(Base64InputStreamFilter &rhs) noexcept {
		using std::swap;
		swap(y_vStream,   rhs.y_vStream);
		swap(x_sbufPlain, rhs.x_sbufPlain);
	}

	friend void swap(Base64InputStreamFilter &lhs, Base64InputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
