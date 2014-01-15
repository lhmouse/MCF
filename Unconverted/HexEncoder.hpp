// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_HEX_ENCODER_HPP__
#define __MCF_HEX_ENCODER_HPP__

#include "StdMCF.hpp"
#include "AbstractTransformerBaseClass.hpp"
#include <vector>

namespace MCF {
	class HexEncoder final : private AbstractTransformerBaseClass {
	private:
		const bool xm_bUpperCase;
	public:
		HexEncoder(bool bUpperCase);
		// ~HexEncoder();
	private:
		virtual void xInitialize(void *pOutput) override;
		virtual void xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize) override;
		virtual void xFinalize(void *pOutput) override;
	public:
		void Transform(std::vector<BYTE> *pvecOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream);
	};
}

#endif
