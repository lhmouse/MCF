// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_STREAM_FILTER_BASE_HPP_
#define MCF_STREAM_FILTERS_STREAM_FILTER_BASE_HPP_

#include "../Core/StreamBuffer.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class StreamFilterBase {
private:
	bool $bInited;
	StreamBuffer $sbufOutput;
	std::uint64_t $u64BytesProcessed;

protected:
	StreamFilterBase() noexcept
		: $bInited(false), $u64BytesProcessed(0)
	{
	}
	virtual ~StreamFilterBase();

protected:
	virtual void $DoInit() = 0;
	virtual void $DoUpdate(const void *pData, std::size_t uSize) = 0;
	virtual void $DoFinalize() = 0;

	// 子类中使用这两个函数输出数据。
	void $Output(unsigned char by){
		$sbufOutput.Put(by);
	}
	void $Output(const void *pData, std::size_t uSize){
		$sbufOutput.Put(pData, uSize);
	}

public:
	void Abort() noexcept {
		if($bInited){
			$sbufOutput.Clear();
			$u64BytesProcessed = 0;

			$bInited = false;
		}
	}
	void Update(const void *pData, std::size_t uSize){
		if(!$bInited){
			$sbufOutput.Clear();
			$u64BytesProcessed = 0;

			$DoInit();

			$bInited = true;
		}

		$DoUpdate(pData, uSize);
		$u64BytesProcessed += uSize;
	}
	void Finalize(){
		if($bInited){
			$DoFinalize();

			$bInited = false;
		}
	}

	std::size_t QueryBytesProcessed() const noexcept {
		return $u64BytesProcessed;
	}
	const StreamBuffer &GetOutputBuffer() const noexcept {
		return $sbufOutput;
	}
	StreamBuffer &GetOutputBuffer() noexcept {
		return $sbufOutput;
	}

	StreamFilterBase &Filter(const StreamBuffer &sbufData){
		ASSERT(&$sbufOutput != &sbufData);

		for(auto ce = sbufData.GetChunkEnumerator(); ce; ++ce){
			Update(ce.GetData(), ce.GetSize());
		}
		return *this;
	}
	StreamFilterBase &FilterInPlace(StreamBuffer &sbufData){
		Filter(sbufData);
		Finalize();
		sbufData.Swap($sbufOutput);

		$sbufOutput.Clear();
		$u64BytesProcessed = 0;
		return *this;
	}
};

}

#endif
