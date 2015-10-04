// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ISAAC_EX_HPP_
#define MCF_STREAM_FILTERS_ISAAC_EX_HPP_

#include "StreamFilterBase.hpp"
#include "../Random/IsaacGenerator.hpp"
#include "../Core/Array.hpp"

namespace MCF {

class IsaacExEncoder : public StreamFilterBase {
private:
	Array<std::uint32_t, 8> x_au32KeyHash;

	IsaacGenerator x_vIsaacGenerator;
	unsigned char x_byLastEncoded;
	long x_lLastHighWord;

public:
	IsaacExEncoder(const void *pKey, std::size_t uKeyLen) noexcept;

protected:
	void X_DoInit() override;
	void X_DoUpdate(const void *pData, std::size_t uSize) override;
	void X_DoFinalize() override;
};

class IsaacExDecoder : public StreamFilterBase {
private:
	Array<std::uint32_t, 8> x_au32KeyHash;

	IsaacGenerator x_vIsaacGenerator;
	unsigned char x_byLastEncoded;
	long x_lLastHighWord;

public:
	IsaacExDecoder(const void *pKey, std::size_t uKeyLen) noexcept;

protected:
	void X_DoInit() override;
	void X_DoUpdate(const void *pData, std::size_t uSize) override;
	void X_DoFinalize() override;
};

}

#endif
