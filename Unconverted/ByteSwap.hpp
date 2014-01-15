// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_BYTE_SWAP_HPP__
#define __MCF_BYTE_SWAP_HPP__

#include "StdMCF.hpp"
#include "AbstractTransformerBaseClass.hpp"

namespace MCF {
	class ByteSwapper final : private AbstractTransformerBaseClass {
	public:
		typedef enum xtagUnit : std::size_t {
			U_WORD = 1,
			U_DWORD,
			U_QWORD
		} UNIT;
	private:
		const UNIT xm_eUnit;
		BYTE xm_abyHistoryBytes[8];
		std::size_t xm_uHistoryByteCount;
	public:
		ByteSwapper(UNIT eUnit);
		// ~ByteSwapper();
	private:
		void xPushByte(std::vector<BYTE> *pvecOutput, BYTE byData);
	private:
		virtual void xInitialize(void *pOutput) override;
		virtual void xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize) override;
		virtual void xFinalize(void *pOutput) override;
	public:
		void Transform(std::vector<BYTE> *pvecOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream);
	};
}

#endif
