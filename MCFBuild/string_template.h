// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_STRING_TEMPLATE_H_
#define MCFBUILD_STRING_TEMPLATE_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

/*
 Syntax
 1. Escape Sequences
    \\          Literal \
    \'          Literal '
    \"          Literal "
    \?          Literal ?
    \$          Literal $
    \#          Literal #
    \a          Alert (0x07)
    \b          Backspace (0x08)
    \f          Form feed (0x0C)
    \n          Line feed (0x0A)
    \r          Carriage return (0x0D)
    \t          Tab (0x09)
    \v          Vertical tab (0x0B)
    \xHH        UTF-16 code unit (2 digits with zero extension)
    \uHHHH      UTF-16 code unit (4 digits)
    \UHHHHHHHH  UTF code point (8 digits)
 2. Replacement Sequences
    $$          Literal $
    $[N]        The Nth (max 9999th) element from the top of the string stack
    $0 ~ $9     The same as $[0] ~ $[9]
    ${KEY}      The value of KEY in the variable map
 3. Comments
    Anything following a non-literal #, as well as the # itself, is ignored.
 4. Single Quotes
    Anything between an innermost pair of single quotes are preserved literally.
 5. Escaped Line Feeds
    If a line feed is escaped, the line preceding it is concatenated with the line following it, with a space delimiter in the middle. The line feed itself is removed.
 6. Whitespaces
    Literal whitespaces (spaces, tabs and line feeds) are used to delimit parameters. Each series made up of literal spaces/tabs is replaced with a single space.
*/

typedef struct MCFBUILD_tagStringStack MCFBUILD_StringStack;
typedef struct MCFBUILD_tagVariableMap MCFBUILD_VariableMap;
typedef struct MCFBUILD_tagNaiveString MCFBUILD_NaiveString;

typedef struct MCFBUILD_tagStringTemplate {
	unsigned char *pbyStorage;
	MCFBUILD_STD size_t uCapacity;
	MCFBUILD_STD size_t uOffsetEnd;
} MCFBUILD_StringTemplate;

typedef enum MCFBUILD_tagStringTemplateParseResult {
	MCFBUILD_kStringTemplateParseSuccess               = 3001,
	MCFBUILD_kStringTemplateParsePartial               = 3002,
	MCFBUILD_kStringTemplateParseEscapeInvalid         = 5001,
	MCFBUILD_kStringTemplateParseDigitsTooFew          = 5002,
	MCFBUILD_kStringTemplateParseDigitsInvalid         = 5003,
	MCFBUILD_kStringTemplateParseReplacementInvalid    = 5004,
	MCFBUILD_kStringTemplateParseBraceUnclosed         = 5005,
	MCFBUILD_kStringTemplateParseSingleQuoteUnclosed   = 5006,
	MCFBUILD_kStringTemplateParseDoubleQuoteUnclosed   = 5007,
	MCFBUILD_kStringTemplateParseUtfCodePointInvalid   = 5008,
	MCFBUILD_kStringTemplateParseNotEnoughMemory       = 5009,
	MCFBUILD_kStringTemplateParseSquareBracketUnclosed = 5010,
	MCFBUILD_kStringTemplateParseStackSubscriptInvalid = 5011,
	MCFBUILD_kStringTemplateParseKeyInvalid            = 5012,
} MCFBUILD_StringTemplateParseResult;

// This function constructs an empty `MCFBUILD_StringTemplate`.
extern void MCFBUILD_StringTemplateConstruct(MCFBUILD_StringTemplate *pTemplate) MCFBUILD_NOEXCEPT;
// This function destructs a `MCFBUILD_StringTemplate`, deallocating any storage it has allocated.
extern void MCFBUILD_StringTemplateDestruct(MCFBUILD_StringTemplate *pTemplate) MCFBUILD_NOEXCEPT;
// This function constructs a `MCFBUILD_StringTemplate` using the contents from `*pSource` and destructs it thereafter.
extern void MCFBUILD_StringTemplateMove(MCFBUILD_StringTemplate *MCFBUILD_RESTRICT pTemplate, MCFBUILD_StringTemplate *MCFBUILD_RESTRICT pSource) MCFBUILD_NOEXCEPT;

extern bool MCFBUILD_StringTemplateIsEmpty(const MCFBUILD_StringTemplate *pTemplate) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_StringTemplateClear(MCFBUILD_StringTemplate *pTemplate) MCFBUILD_NOEXCEPT;
// Any elements parsed are appended to `*pTemplate`. If the function fails, the contents of `*pTemplate` are left intact.
extern bool MCFBUILD_StringTemplateParse(MCFBUILD_StringTemplate *MCFBUILD_RESTRICT pTemplate, MCFBUILD_StringTemplateParseResult *MCFBUILD_RESTRICT peResult, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puResultOffset, const wchar_t *MCFBUILD_RESTRICT pwszRawString) MCFBUILD_NOEXCEPT;

extern bool MCFBUILD_StringTemplateBuildStringVector(MCFBUILD_NaiveString **MCFBUILD_RESTRICT ppStringVector, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puStringCount, const MCFBUILD_StringTemplate *MCFBUILD_RESTRICT pTemplate, const MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack, const MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_StringTemplateFreeStringVector(MCFBUILD_NaiveString *pStringVector) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
