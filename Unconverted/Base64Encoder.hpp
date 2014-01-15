// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_BASE64_ENCODER_HPP__
#define __MCF_BASE64_ENCODER_HPP__

#include "StdMCF.hpp"
#include "AbstractTransformerBaseClass.hpp"
#include "xBase64CodecBase.hpp"
#include <vector>

namespace MCF {
	class Base64Encoder final : public xBase64CodecBase {
	private:
		static const BYTE xENCODE_TABLE_DRAFT[0x40];
	private:
		BYTE xm_abyEncodeTable[0x40];
		BYTE xm_byPadding;

		enum {
			INIT,
			BYTE_1,
			BYTE_2
		} xm_eState;
		DWORD xm_dwHistory;
	public:
		Base64Encoder(const PARAMS &Params = B64P_MIME);
		~Base64Encoder();
	private:
		virtual void xInitialize(void *pOutput) override;
		virtual void xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize) override;
		virtual void xFinalize(void *pOutput) override;
	public:
		void Transform(std::vector<BYTE> *pvecOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream);
	};
}

#endif __BASE64_ENCODER_HPP__
