// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_ISAAC_EX_FILTERS_HPP_
#define MCF_ISAAC_EX_FILTERS_HPP_

#include "StreamFilterBase.hpp"
#include <memory>

namespace MCF {

class IsaacRng;

class IsaacExEncoder : CONCRETE(StreamFilterBase) {
private:
	union {
		unsigned char aby[32];
		std::uint32_t au32[8];
	} xm_vKeyHash;

	std::unique_ptr<IsaacRng> xm_pIsaacRng;
	unsigned char xm_abyLastEncoded;

public:
	IsaacExEncoder(const void *pKey, std::size_t uKeyLen) noexcept;
	~IsaacExEncoder() noexcept;

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

class IsaacExDecoder : CONCRETE(StreamFilterBase) {
private:
	union {
		unsigned char aby[32];
		std::uint32_t au32[8];
	} xm_vKeyHash;

	std::unique_ptr<IsaacRng> xm_pIsaacRng;
	unsigned char xm_abyLastEncoded;

public:
	IsaacExDecoder(const void *pKey, std::size_t uKeyLen) noexcept;
	~IsaacExDecoder() noexcept;

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

}

#endif
