// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BASE64_HPP_
#define MCF_STREAM_FILTERS_BASE64_HPP_

#include "StreamFilterBase.hpp"

namespace MCF {

constexpr char kBase64TableMime   [66] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
constexpr char kBase64TableUrl    [66] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789*-";
constexpr char kBase64TableRegexp [66] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!-=";

class Base64Encoder : public StreamFilterBase {
private:
	unsigned char x_abyTable[65];

	unsigned x_uState;
	unsigned long x_ulWord;

public:
	explicit Base64Encoder(const char *pchTable = kBase64TableMime) noexcept;

protected:
	void X_DoInit() override;
	void X_DoUpdate(const void *pData, std::size_t uSize) override;
	void X_DoFinalize() override;
};

class Base64Decoder : public StreamFilterBase {
private:
	signed char x_aschTable[0x100];

	unsigned x_uState;
	unsigned long x_ulWord;

public:
	explicit Base64Decoder(const char *pchTable = kBase64TableMime) noexcept;

protected:
	void X_DoInit() override;
	void X_DoUpdate(const void *pData, std::size_t uSize) override;
	void X_DoFinalize() override;
};

}

#endif
