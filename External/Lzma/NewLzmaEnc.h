// Public domain, LH_Mouse. All wrongs reserved.

#ifndef __NEW_LZMA_ENC_H
#define __NEW_LZMA_ENC_H

#include "LzmaEnc.h"

#ifdef __cplusplus
extern "C" {
#endif

void *LzmaEnc_NewEncodeCreateContext(CLzmaEncHandle p, ISeqOutStream *outStream, ICompressProgress *progress, ISzAlloc *alloc, ISzAlloc *allocBig);
void LzmaEnc_NewEncodeDestroyContext(void *ctx);

SRes LzmaEnc_NewEncode(void *ctx, const Byte *src, SizeT srcLen);

#ifdef __cplusplus
}
#endif

#endif
