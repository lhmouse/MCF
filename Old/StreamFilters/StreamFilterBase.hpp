// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_STREAM_FILTER_BASE_HPP_
#define MCF_STREAM_FILTERS_STREAM_FILTER_BASE_HPP_

#include "../Core/StreamBuffer.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class StreamFilterBase {
private:
	bool x_bInited;
	StreamBuffer x_sbufOutput;
	std::uint64_t x_u64BytesProcessed;

protected:
	constexpr StreamFilterBase() noexcept
		: x_bInited(false), x_sbufOutput(), x_u64BytesProcessed(0)
	{
	}
	virtual ~StreamFilterBase();

protected:
	virtual void X_DoInit() = 0;
	virtual void X_DoUpdate(const void *pData, std::size_t uSize) = 0;
	virtual void X_DoFinalize() = 0;

	// 子类中使用这两个函数输出数据。
	void X_Output(unsigned char by){
		x_sbufOutput.Put(by);
	}
	void X_Output(const void *pData, std::size_t uSize){
		x_sbufOutput.Put(pData, uSize);
	}

public:
	void Abort() noexcept {
		if(x_bInited){
			x_sbufOutput.Clear();
			x_u64BytesProcessed = 0;

			x_bInited = false;
		}
	}
	void Update(const void *pData, std::size_t uSize){
		if(!x_bInited){
			x_sbufOutput.Clear();
			x_u64BytesProcessed = 0;

			X_DoInit();

			x_bInited = true;
		}

		X_DoUpdate(pData, uSize);
		x_u64BytesProcessed += uSize;
	}
	StreamBuffer &Finalize(){
		if(x_bInited){
			X_DoFinalize();

			x_bInited = false;
		}

		return GetOutputBuffer();
	}

	std::uint64_t QueryBytesProcessed() const noexcept {
		return x_u64BytesProcessed;
	}
	const StreamBuffer &GetOutputBuffer() const noexcept {
		return x_sbufOutput;
	}
	StreamBuffer &GetOutputBuffer() noexcept {
		return x_sbufOutput;
	}

	StreamFilterBase &Filter(const StreamBuffer &sbufData){
		ASSERT(&x_sbufOutput != &sbufData);

		sbufData.Iterate([this](auto pbyData, auto uSize){ this->Update(pbyData, uSize); });
		return *this;
	}
	StreamFilterBase &FilterInPlace(StreamBuffer &sbufData){
		Filter(sbufData);
		Finalize();
		sbufData.Swap(x_sbufOutput);

		x_sbufOutput.Clear();
		x_u64BytesProcessed = 0;
		return *this;
	}
};

}

#endif
