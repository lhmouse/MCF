// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_MT19937_CLASS_HPP__
#define __MCF_MT19937_CLASS_HPP__

#include "StdMCF.hpp"

namespace MCF {
	class MT19937Class {
	private:
		static const DWORD xGEN_FLIP_MASK[4];
		static const DWORD xEXTRACT_MASK1[4];
		static const DWORD xEXTRACT_MASK2[4];
	private:
		__declspec(align(16)) DWORD xm_ardwMT[624];
		__declspec(align(16)) DWORD xm_ardwResults[624];
		std::size_t xm_uIndex;
	public:
		MT19937Class(DWORD dwSeed);
		~MT19937Class();
	private:
		void __thiscall xGenNumbers();
	public:
		void __fastcall Init(DWORD dwSeed);
		DWORD Get();
	};
}

#endif
