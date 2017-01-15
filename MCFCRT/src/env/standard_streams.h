// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_STANDARD_STREAMS_H_
#define __MCFCRT_ENV_STANDARD_STREAMS_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_StandardStreamsInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_StandardStreamsUninit(void) _MCFCRT_NOEXCEPT;

// 这些函数使用 UTF 编码。如果操作的是控制台，以 UTF-16 作为内码；否则，假定操作的是字节流，以 UTF-8 作为内码。
// 对于返回 int 或 long 的函数，失败返回 -1；对于返回 size_t 的函数，失败返回 0（如果读取到流结尾，GetLastError() 返回 ERROR_HANDLE_EOF）。
// 操作文本的函数保证不会返回不完整的码点。

// 标准输入流二进制输入。
extern int _MCFCRT_PeekStandardInputByte(void) _MCFCRT_NOEXCEPT;
extern int _MCFCRT_ReadStandardInputByte(void) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_PeekStandardInputBinary(void *_MCFCRT_RESTRICT __pData, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_ReadStandardInputBinary(void *_MCFCRT_RESTRICT __pData, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_DiscardStandardInputBinary(_MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
// 标准输入流文本输入。
extern long _MCFCRT_PeekStandardInputChar32(void) _MCFCRT_NOEXCEPT;
extern long _MCFCRT_ReadStandardInputChar32(void) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_PeekStandardInputText(wchar_t *_MCFCRT_RESTRICT __pwcText, _MCFCRT_STD size_t __uLength, bool __bSingleLine) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_ReadStandardInputText(wchar_t *_MCFCRT_RESTRICT __pwcText, _MCFCRT_STD size_t __uLength, bool __bSingleLine) _MCFCRT_NOEXCEPT;
extern _MCFCRT_STD size_t _MCFCRT_DiscardStandardInputText(_MCFCRT_STD size_t __uLength, bool __bSingleLine) _MCFCRT_NOEXCEPT;
// 标准输入流控制。
extern bool _MCFCRT_IsStandardInputEchoing(void) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_SetStandardInputEchoing(bool __bEchoing) _MCFCRT_NOEXCEPT;

// 标准输出流二进制输出。
extern bool _MCFCRT_WriteStandardOutputByte(unsigned char __byData) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardOutputBinary(const void *_MCFCRT_RESTRICT __pData, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
// 标准输出流文本输出。
extern bool _MCFCRT_WriteStandardOutputChar32(char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardOutputText(const wchar_t *_MCFCRT_RESTRICT __pwcText, _MCFCRT_STD size_t __uLength, bool __bAppendNewLine) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_IsStandardOutputBuffered(void) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SetStandardOutputBuffered(bool __bBuffered) _MCFCRT_NOEXCEPT;
// 标准输出流控制。
extern bool _MCFCRT_FlushStandardOutput(bool __bHard) _MCFCRT_NOEXCEPT;

// 标准错误流二进制输出。
extern bool _MCFCRT_WriteStandardErrorByte(unsigned char __byData) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardErrorBinary(const void *_MCFCRT_RESTRICT __pData, _MCFCRT_STD size_t __uSize) _MCFCRT_NOEXCEPT;
// 标准错误流文本输出。
extern bool _MCFCRT_WriteStandardErrorChar32(char32_t __c32CodePoint) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_WriteStandardErrorText(const wchar_t *_MCFCRT_RESTRICT __pwcText, _MCFCRT_STD size_t __uLength, bool __bAppendNewLine) _MCFCRT_NOEXCEPT;
// 标准错误流控制。
extern bool _MCFCRT_FlushStandardError(bool __bHard) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
