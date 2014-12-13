// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_STREAM_FILTER_BASE_HPP_
#define MCF_STREAM_FILTERS_STREAM_FILTER_BASE_HPP_

#include "../Core/StreamBuffer.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class StreamFilterBase {
private:
	bool xm_bInited;
	StreamBuffer xm_sbufOutput;
	std::uint64_t xm_u64BytesProcessed;

protected:
	StreamFilterBase() noexcept
		: xm_bInited(false), xm_u64BytesProcessed(0)
	{
	}
	virtual ~StreamFilterBase();

protected:
	virtual void xDoInit() = 0;
	virtual void xDoUpdate(const void *pData, std::size_t uSize) = 0;
	virtual void xDoFinalize() = 0;

	// 子类中使用这两个函数输出数据。
	void xOutput(unsigned char by){
		xm_sbufOutput.Put(by);
	}
	void xOutput(const void *pData, std::size_t uSize){
		xm_sbufOutput.Put(pData, uSize);
	}

public:
	void Abort() noexcept {
		if(xm_bInited){
			xm_sbufOutput.Clear();
			xm_u64BytesProcessed = 0;

			xm_bInited = false;
		}
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_bInited){
			xm_sbufOutput.Clear();
			xm_u64BytesProcessed = 0;

			xDoInit();

			xm_bInited = true;
		}

		xDoUpdate(pData, uSize);
		xm_u64BytesProcessed += uSize;
	}
	void Finalize(){
		if(xm_bInited){
			xDoFinalize();

			xm_bInited = false;
		}
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
