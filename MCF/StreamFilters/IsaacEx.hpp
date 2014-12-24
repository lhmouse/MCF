// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ISAAC_EX_HPP_
#define MCF_STREAM_FILTERS_ISAAC_EX_HPP_

#include "StreamFilterBase.hpp"
#include "../Random/IsaacGenerator.hpp"

namespace MCF {

struct IsaacFilterKeyHash {
	std::uint32_t au32Words[8];
};

class IsaacExEncoder : public StreamFilterBase {
private:
	IsaacFilterKeyHash xm_vKeyHash;

	IsaacGenerator xm_vIsaacGenerator;
	unsigned char xm_byLastEncoded;
	long xm_lLastHighWord;

public:
	IsaacExEncoder(const void *pKey, std::size_t uKeyLen) noexcept;

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

class IsaacExDecoder : public StreamFilterBase {
private:
	IsaacFilterKeyHash xm_vKeyHash;

	IsaacGenerator xm_vIsaacGenerator;
	unsigned char xm_byLastEncoded;
	long xm_lLastHighWord;

public:
	IsaacExDecoder(const void *pKey, std::size_t uKeyLen) noexcept;

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

}

#endif
