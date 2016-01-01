// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_LZMA_HPP_
#define MCF_STREAM_FILTERS_LZMA_HPP_

#include "StreamFilterBase.hpp"
#include "../Core/Exception.hpp"
#include "../SmartPointers/UniquePtr.hpp"

namespace MCF {

class LzmaEncoder : public StreamFilterBase {
private:
	class X_Delegate;

private:
	const unsigned x_uLevel;
	const unsigned long x_ulDictSize;

	UniquePtr<X_Delegate> x_pDelegate;

public:
	explicit LzmaEncoder(unsigned uLevel = 6, unsigned long ulDictSize = 1ul << 23) noexcept;
	~LzmaEncoder();

protected:
	void X_DoInit() override;
	void X_DoUpdate(const void *pData, std::size_t uSize) override;
	void X_DoFinalize() override;
};

class LzmaDecoder : public StreamFilterBase {
private:
	class X_Delegate;

private:
	UniquePtr<X_Delegate> x_pDelegate;

public:
	LzmaDecoder() noexcept;
	~LzmaDecoder();

protected:
	void X_DoInit() override;
	void X_DoUpdate(const void *pData, std::size_t uSize) override;
	void X_DoFinalize() override;
};

class LzmaError : public Exception {
private:
	long x_lLzmaError;

public:
	LzmaError(const char *pszFile, unsigned long ulLine, long lLzmaError, RefCountingNtmbs rcsFunction) noexcept;
	~LzmaError() override;

public:
	long GetLzmaError() const noexcept {
		return x_lLzmaError;
	}
};

}

#endif
