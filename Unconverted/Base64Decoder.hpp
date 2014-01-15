// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_BASE64_DECODER_HPP__
#define __MCF_BASE64_DECODER_HPP__

#include "StdMCF.hpp"
#include "AbstractTransformerBaseClass.hpp"
#include "xBase64CodecBase.hpp"
#include <vector>

namespace MCF {
	class Base64Decoder final : public xBase64CodecBase {
	private:
		static const BYTE xDECODE_TABLE_DRAFT[0x100];
	private:
		BYTE xm_abyDecodeTable[0x100];

		enum {
			INIT,
			BYTE_1,
			BYTE_2,
			BYTE_3
		} xm_eState;
		DWORD xm_dwHistory;
	public:
		Base64Decoder(const PARAMS &Params = B64P_MIME);
		~Base64Decoder();
	private:
		virtual void xInitialize(void *pOutput) override;
		virtual void xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize) override;
		virtual void xFinalize(void *pOutput) override;
	public:
		void Transform(std::vector<BYTE> *pvecOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream);
	};
}

#endif
