// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SHA256_HPP__
#define __MCF_SHA256_HPP__

#include "StdMCF.hpp"
#include <vector>
#include "AbstractTransformerBaseClass.hpp"

namespace MCF {
	class SHA256Hasher : public AbstractTransformerBaseClass {
	private:
		static void __fastcall xHashChunk(DWORD (*pardwOutput)[8], const BYTE (*pabyChunk)[64]);
	private:
		BYTE xm_abyLastChunk[64];
		std::size_t xm_uBytesInLastChunk;
		std::uint64_t xm_uBytesTotal;
	public:
		// SHA256Hasher();
		// ~SHA256Hasher();
	private:
		virtual void xInitialize(void *pOutput) override;
		virtual void xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize) override;
		virtual void xFinalize(void *pOutput) override;
	public:
		void Transform(BYTE (*pardwOutput)[32], const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream);
	};
}

#endif
