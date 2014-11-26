// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_STREAM_FILTER_BASE_HPP_
#define MCF_STREAM_FILTERS_STREAM_FILTER_BASE_HPP_

#include "../Core/StreamBuffer.hpp"
#include "../Utilities/NoCopy.hpp"
#include "../Utilities/Abstract.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class StreamFilterBase : NO_COPY, ABSTRACT {
private:
	StreamBuffer xm_sbufOutput;
	std::uint64_t xm_u64BytesProcessed;

protected:
	StreamFilterBase() noexcept
		: xm_u64BytesProcessed(0)
	{
	}

protected:
	void xOutput(unsigned char by){
		xm_sbufOutput.Put(by);
	}
	void xOutput(const void *pData, std::size_t uSize){
		xm_sbufOutput.Put(pData, uSize);
	}

public:
	virtual void Abort() noexcept {
		xm_sbufOutput.Clear();
		xm_u64BytesProcessed = 0;
	}
	virtual void Update(const void *, std::size_t uSize){
		xm_u64BytesProcessed += uSize;
	}
	virtual void Finalize(){
	}

	std::size_t QueryBytesProcessed() const noexcept {
		return xm_u64BytesProcessed;
	}
	const StreamBuffer &GetOutputBuffer() const noexcept {
		return xm_sbufOutput;
	}
	StreamBuffer &GetOutputBuffer() noexcept {
		return xm_sbufOutput;
	}

	StreamFilterBase &Filter(const StreamBuffer &sbufData){
		ASSERT(&xm_sbufOutput != &sbufData);

		sbufData.Traverse([this](auto pbyData, auto uSize){ this->Update(pbyData, uSize); });
		return *this;
	}
	StreamFilterBase &FilterInPlace(StreamBuffer &sbufData){
		Filter(sbufData);
		Finalize();
		sbufData.Swap(xm_sbufOutput);

		xm_sbufOutput.Clear();
		xm_u64BytesProcessed = 0;
		return *this;
	}
};

}

#endif
