// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_FILE_H_
#define MCFBUILD_FILE_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

// Be noted that no file size shall exceed 4GiB.
// Otherwise these functions fail and `GetLastError()` returns `ERROR_ARITHMETIC_OVERFLOW`.

// Upon success, `MCFBUILD_FileGetContents()` allocates a buffer, stores a pointer to the buffer into `*ppData` and the size of the buffer into `*puSize`.
// The buffer shall be freed with `MCFBUILD_FileFreeContents()`.
extern bool MCFBUILD_FileGetContents(void *MCFBUILD_RESTRICT *MCFBUILD_RESTRICT ppData, MCFBUILD_STD size_t *puSize, const wchar_t *MCFBUILD_RESTRICT pwcPath) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_FileFreeContents(void *pData) MCFBUILD_NOEXCEPT;

// Upon failure, the contents of the file are indeterminate.
extern bool MCFBUILD_FilePutContents(const wchar_t *pwcPath, const void *pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_FileAppendContents(const wchar_t *pwcPath, const void *pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
