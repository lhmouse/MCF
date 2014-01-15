// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRC32__HPP__
#define __MCF_CRC32__HPP__

#include "StdMCF.hpp"
#include "AbstractTransformerBaseClass.hpp"

namespace MCF {
	class CRC32Hasher final : private AbstractTransformerBaseClass {
	public:
		static const DWORD DIVISOR_IEEE_802_3	= 0xEDB88320;
		static const DWORD DIVISOR_CASTAGNOLI	= 0x82F63B78;
	private:
		static void __fastcall xBuildTable(DWORD (*pardwTable)[0x100], DWORD dwDivisor);
	private:
		DWORD xm_ardwTable[0x100];
	public:
		CRC32Hasher(DWORD dwDivisor = DIVISOR_IEEE_802_3);
		// ~CRC32Hasher();
	private:
		virtual void xInitialize(void *pOutput) override;
		virtual void xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize) override;
		virtual void xFinalize(void *pOutput) override;
	public:
		void Transform(DWORD *pdwOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream);
	};
}

#endif
