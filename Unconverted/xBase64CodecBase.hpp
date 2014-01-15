// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_BASE64_CODEC_BASE_HPP__
#define __MCF_X_BASE64_CODEC_BASE_HPP__

#include "StdMCF.hpp"
#include "AbstractTransformerBaseClass.hpp"

namespace MCF {
	class xBase64CodecBase : protected AbstractTransformerBaseClass {
	public:
		typedef struct xtagBase64Params {
			char ch63rd;
			char ch64th;
			char chPadding;
		} PARAMS;
	public:
		static const PARAMS B64P_MIME;
		static const PARAMS B64P_URL;
		static const PARAMS B64P_REGEXP;
	protected:
		// xBase64CodecBase();
		// ~xBase64CodecBase();
	};
}

#endif
