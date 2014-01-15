// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_ABSTRACT_TRANSFORMER_BASE_CLASS_HPP__
#define __MCF_ABSTRACT_TRANSFORMER_BASE_CLASS_HPP__

#include "StdMCF.hpp"
#include <vector>

namespace MCF {
	template class std::vector<BYTE>;

	class AbstractTransformerBaseClass : NO_COPY_OR_ASSIGN {
	private:
		bool xm_bStreamEnded;
	protected:
		AbstractTransformerBaseClass();
		virtual ~AbstractTransformerBaseClass();
	private:
		virtual void xInitialize(void *pOutput) = 0;
		virtual void xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize) = 0;
		virtual void xFinalize(void *pOutput) = 0;
	protected:
		void xTransform(void *pOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream);
	};
}

#endif
