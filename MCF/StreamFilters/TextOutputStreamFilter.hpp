// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_TEXT_OUTPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_TEXT_OUTPUT_STREAM_FILTER_HPP_

#include "AbstractOutputStreamFilter.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

class TextOutputStreamFilter : public AbstractOutputStreamFilter {
private:
	Vector<char> x_vecEncoded;

public:
	explicit TextOutputStreamFilter(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream) noexcept
		: AbstractOutputStreamFilter(std::move(pUnderlyingStream))
	{
	}
	~TextOutputStreamFilter() override;

	TextOutputStreamFilter(TextOutputStreamFilter &&) noexcept = default;
	TextOutputStreamFilter& operator=(TextOutputStreamFilter &&) noexcept = default;

private:
	void X_FlushEncodedBuffer();

public:
	void Put(unsigned char byData) override;

	void Put(const void *pData, std::size_t uSize) override;

	void Flush(bool bHard) override;

	void Swap(TextOutputStreamFilter &rhs) noexcept {
		using std::swap;
		swap(y_vStream,    rhs.y_vStream);
		swap(x_vecEncoded, rhs.x_vecEncoded);
	}

	friend void swap(TextOutputStreamFilter &lhs, TextOutputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
