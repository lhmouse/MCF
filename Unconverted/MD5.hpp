// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_MD5_HPP__
#define __MCF_MD5_HPP__

#include "StdMCF.hpp"
#include "AbstractTransformerBaseClass.hpp"

namespace MCF {
	class MD5Hasher final : private AbstractTransformerBaseClass {
	private:
		static void __stdcall xHashChunk(DWORD (*pardwOutput)[4], const BYTE (*pabyChunk)[64]);
	private:
		BYTE xm_abyLastChunk[64];
		std::size_t xm_uBytesInLastChunk;
		std::uint64_t xm_uBytesTotal;
	public:
		// MD5Hasher();
		// ~MD5Hasher();
	private:
		virtual void xInitialize(void *pOutput) override;
		virtual void xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize) override;
		virtual void xFinalize(void *pOutput) override;
	public:
		void Transform(BYTE (*pardwOutput)[16], const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream);
	};
}

#endif
