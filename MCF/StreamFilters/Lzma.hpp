// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_LZMA_HPP_
#define MCF_STREAM_FILTERS_LZMA_HPP_

#include "StreamFilterBase.hpp"
#include "../Core/Exception.hpp"
#include "../SmartPointers/UniquePtr.hpp"

namespace MCF {

class LzmaEncoder : public StreamFilterBase {
private:
	class $Delegate;

private:
	const unsigned $uLevel;
	const unsigned long $ulDictSize;

	UniquePtr<$Delegate> $pDelegate;

public:
	explicit LzmaEncoder(unsigned uLevel = 6, unsigned long ulDictSize = 1ul << 23) noexcept;
	~LzmaEncoder();

protected:
	void $DoInit() override;
	void $DoUpdate(const void *pData, std::size_t uSize) override;
	void $DoFinalize() override;
};

class LzmaDecoder : public StreamFilterBase {
private:
	class $Delegate;

private:
	UniquePtr<$Delegate> $pDelegate;

public:
	LzmaDecoder() noexcept;
	~LzmaDecoder();

protected:
	void $DoInit() override;
	void $DoUpdate(const void *pData, std::size_t uSize) override;
	void $DoFinalize() override;
};

class LzmaError : public Exception {
private:
	long $lLzmaError;

public:
	LzmaError(const char *pszFile, unsigned long ulLine, long lLzmaError, const char *pszFunction) noexcept;
	~LzmaError() override;

public:
	long GetLzmaError() const noexcept {
		return $lLzmaError;
	}
};

}

#endif
