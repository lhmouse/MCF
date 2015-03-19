// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BASE64_HPP_
#define MCF_STREAM_FILTERS_BASE64_HPP_

#include "StreamFilterBase.hpp"

namespace MCF {

constexpr char BASE64_TABLE_MIME	[66] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
constexpr char BASE64_TABLE_URL		[66] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789*-";
constexpr char BASE64_TABLE_REGEXP	[66] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!-=";

class Base64Encoder : public StreamFilterBase {
private:
	unsigned char x_abyTable[65];

	unsigned x_uState;
	unsigned long x_ulWord;

public:
	explicit Base64Encoder(const char *pchTable = BASE64_TABLE_MIME) noexcept;

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

class Base64Decoder : public StreamFilterBase {
private:
	signed char x_aschTable[0x100];

	unsigned x_uState;
	unsigned long x_ulWord;

public:
	explicit Base64Decoder(const char *pchTable = BASE64_TABLE_MIME) noexcept;

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

}

#endif
