// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_RC4EX_FILTERS_HPP_
#define MCF_RC4EX_FILTERS_HPP_

#include "StreamFilterBase.hpp"

namespace MCF {

class RC4ExFilterBase : CONCRETE(StreamFilterBase) {
private:
	unsigned char xm_abyInitBox[256];

protected:
	unsigned char xm_abyBox[256];
	unsigned char xm_i;
	unsigned char xm_j;

protected:
	RC4ExFilterBase(const void *pKey, std::size_t uKeyLen) noexcept;

protected:
	void xInit() noexcept;

public:
	void Abort() noexcept override;
	void Finalize() override;
};

class RC4ExEncoder : CONCRETE(RC4ExFilterBase) {
public:
	RC4ExEncoder(const void *pKey, std::size_t uKeyLen) noexcept;

public:
	void Update(const void *pData, std::size_t uSize) override;
};

class RC4ExDecoder : CONCRETE(RC4ExFilterBase) {
public:
	RC4ExDecoder(const void *pKey, std::size_t uKeyLen) noexcept;

public:
	void Update(const void *pData, std::size_t uSize) override;
};

}

#endif
