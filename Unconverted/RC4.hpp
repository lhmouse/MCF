// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_RC4_HPP__
#define __MCF_RC4_HPP__

#include "StdMCF.hpp"
#include "AbstractTransformerBaseClass.hpp"
#include <vector>

namespace MCF {
	class RC4Encoder final : private AbstractTransformerBaseClass {
	private:
		const BYTE *xm_pbyKey;
		std::size_t xm_uKeyLen;
		BYTE xm_abyBox[256];
		std::vector<BYTE> xm_vecbyTemp;

		std::size_t xm_uContextI;
		std::size_t xm_uContextJ;
	public:
		// RC4Encoder();
		// ~RC4Encoder();
	private:
		virtual void xInitialize(void *pOutput) override;
		virtual void xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize) override;
		virtual void xFinalize(void *pOutput) override;
	public:
		void Transform(std::vector<BYTE> *pvecbyOutput, const BYTE *pbyKey, std::size_t uKeyLen, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream);
	};

	typedef RC4Encoder RC4Decoder;
}

#endif
