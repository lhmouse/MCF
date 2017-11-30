// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_FILE_H_
#define MCFBUILD_FILE_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

typedef MCFBUILD_STD uint8_t MCFBUILD_Sha256[32];

// Upon success, `MCFBUILD_FileGetContents()` allocates a buffer, stores a pointer to the buffer into `*ppData` and the size of the buffer into `*puSize`.
// Contents of the buffer are terminated with four extra zero bytes, making it possible to treat the buffer as a null-terminated string.
// `*ppData` shall be freed with `MCFBUILD_FileFreeContentBuffer()`.
extern bool MCFBUILD_FileGetContents(void **MCFBUILD_RESTRICT ppData, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puSize, const wchar_t *pwcPath) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_FileFreeContentBuffer(void *pData) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_FileGetSha256(MCFBUILD_Sha256 *pau8Sha256, const wchar_t *pwcPath) MCFBUILD_NOEXCEPT;

// Upon failure, the contents of the file are indeterminate.
extern bool MCFBUILD_FilePutContents(const wchar_t *pwcPath, const void *pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_FileAppendContents(const wchar_t *pwcPath, const void *pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;

// These functions provide exclusive access semantics.
extern bool MCFBUILD_FileLock(MCFBUILD_STD uintptr_t *MCFBUILD_RESTRICT puCookie, const wchar_t *pwcPath) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_FileUnlock(MCFBUILD_STD uintptr_t uCookie) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
