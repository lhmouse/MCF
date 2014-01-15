// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "xBase64CodecBase.hpp"
using namespace MCF;

// 静态成员变量。
const xBase64CodecBase::PARAMS	xBase64CodecBase::B64P_MIME		= { '+', '/', '=' };
const xBase64CodecBase::PARAMS	xBase64CodecBase::B64P_URL		= { '*', '-',  0  };
const xBase64CodecBase::PARAMS	xBase64CodecBase::B64P_REGEXP	= { '!', '-', '=' };
