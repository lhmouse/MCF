// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_BASE64_FILTERS_HPP_
#define MCF_BASE64_FILTERS_HPP_

#include "StreamFilterBase.hpp"

namespace MCF {

constexpr char BASE64_TABLE_MIME	[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
constexpr char BASE64_TABLE_URL		[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789*-";
constexpr char BASE64_TABLE_REGEXP	[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!-=";

class Base64Encoder : CONCRETE(StreamFilterBase) {
private:
	unsigned char xm_abyTable[65];

	unsigned int xm_uWord;
	unsigned int xm_uState;

public:
	explicit Base64Encoder(const char *pchTable = BASE64_TABLE_MIME) noexcept;

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

class Base64Decoder : CONCRETE(StreamFilterBase) {
private:
	unsigned char xm_abyTable[0x100];

	unsigned int xm_uWord;
	unsigned int xm_uState;

public:
	explicit Base64Decoder(const char *pchTable = BASE64_TABLE_MIME) noexcept;

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

}

#endif
