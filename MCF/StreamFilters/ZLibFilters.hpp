// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_ZLIB_FILTERS_HPP_
#define MCF_ZLIB_FILTERS_HPP_

#include "StreamFilterBase.hpp"
#include <memory>

namespace MCF {

class ZLibEncoder : CONCRETE(StreamFilterBase) {
private:
	class xDelegate;

private:
	const std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit ZLibEncoder(bool bRaw = false, unsigned uLevel = 6);
	~ZLibEncoder() noexcept;

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

class ZLibDecoder : CONCRETE(StreamFilterBase) {
private:
	class xDelegate;

private:
	const std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit ZLibDecoder(bool bRaw = false);
	~ZLibDecoder() noexcept;

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

}

#endif
