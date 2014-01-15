// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_GENERIC_STRING_HPP__
#define __MCF_GENERIC_STRING_HPP__

#include "Utilities.hpp"
#include <tchar.h>
#include <windows.h>
#include <climits>
#include <cstdio>
#include <cwchar>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <string>
#include <memory>
#include <utility>
#include <type_traits>

#ifdef NO_IMPLICIT_CHARSET_CONVERSION_WARNING
#	define xDECL_DEPRECATED_IMPLICIT_CHARSET_CONVERSION
#else
#	define xDECL_DEPRECATED_IMPLICIT_CHARSET_CONVERSION		__attribute__((deprecated("Implicit use of charset conversion is deprecated. Consider using TO_TYPE::ConvFrom<FROM_TYPE>() instead. To disable this warning, define NO_IMPLICIT_CHARSET_CONVERSION_WARNING.")))
#endif

namespace MCF {
	typedef enum {
		ENC_ANSI,
		ENC_UCS2LE,
		ENC_UTF8
	} STR_ENCODING;

	class xGenericStringBase {
	protected:
		typedef std::wstring xUniString;
	protected:
		xGenericStringBase(){ }
		virtual ~xGenericStringBase(){ }
	public:
		virtual xUniString xMy_Unify() const = 0;
	};

	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	class GenericString final : public xGenericStringBase {
	private:
		typedef CHAR_TYPE		xMY_CHAR;
		typedef const xMY_CHAR	*xMY_PCSTR;
		typedef xMY_CHAR		*xMY_PSTR;

		static const STR_ENCODING xMY_ENCODING = ENCODING;
	public:
		static const std::ptrdiff_t	NOT_FOUND = -1;
	private:
		// 以下函数需要在实例化类中定义。
		static std::size_t xMy_len(xMY_PCSTR str);
		static bool xMy_isspace(xMY_CHAR ch);
		static int xMy_cmp(xMY_PCSTR str1, xMY_PCSTR str2);
		static int xMy_ncmp(xMY_PCSTR str1, xMY_PCSTR str2, std::size_t cnt);
		static int xMy_vsnprintf(xMY_PSTR dst, std::size_t cap, xMY_PCSTR fmt, std::va_list ArgList);

		virtual xUniString xMy_Unify() const override;
		static GenericString xMy_Deunify(const xUniString &src);
	private:
		static inline __attribute__((always_inline)) bool xOverlapChk(const void *p1, std::size_t cb1, const void *p2, std::size_t cb2){
			if((p1 == nullptr) || (p2 == nullptr)){
				return false;
			}
			return !(((((const char *)p1) + cb1) <= (const char *)p2) || ((((const char *)p2) + cb2) <= (const char *)p1));
		}
	public:
		// 配合 decltype() 使用。
		static xMY_CHAR GetCharType(){
			return 0;
		}
		static STR_ENCODING GetEncoding(){
			return xMY_ENCODING;
		}
	private:
		xMY_CHAR xm_InBuff[16 / sizeof(xMY_CHAR)];
		xMY_CHAR *xm_ExBuff;
		std::size_t xm_uExBuffCap;
		mutable std::size_t xm_uCachedLen;
	public:
		GenericString(){
			xInit();
		}
		GenericString(xMY_PCSTR pSrc){
			xInit();
			Assign(pSrc, (pSrc != nullptr) ? xMy_len(pSrc) : 0);
		}
		GenericString(xMY_PCSTR pSrc, std::size_t uLen){
			xInit();
			Assign(pSrc, uLen);
		}
		GenericString(xMY_PCSTR pBegin, xMY_PCSTR pEnd){
			xInit();
			Assign(pBegin, pEnd);
		}
		GenericString(xMY_CHAR ch){
			xInit();
			*this = ch;
		}
		GenericString(const GenericString &src){
			xInit();
			*this = src;
		}
		GenericString(GenericString &&src){
			xInit();
			*this = std::move(src);
		}
		explicit xDECL_DEPRECATED_IMPLICIT_CHARSET_CONVERSION GenericString(const xGenericStringBase &src){
			xInit();
			ConvFrom(src);
		}

		GenericString(xMY_PCSTR pLSrc, const GenericString &strSrc, xMY_PCSTR pRSrc){
			xInit();
			xJoinConstruct(pLSrc, (pLSrc != nullptr) ? xMy_len(pLSrc) : 0, strSrc, pRSrc, (pRSrc != nullptr) ? xMy_len(pRSrc) : 0);
		}
		GenericString(xMY_PCSTR pLSrc, std::size_t uLLen, const GenericString &strSrc, xMY_PCSTR pRSrc){
			xInit();
			xJoinConstruct(pLSrc, uLLen, strSrc, pRSrc, (pRSrc != nullptr) ? xMy_len(pRSrc) : 0);
		}
		GenericString(xMY_PCSTR pLSrc, const GenericString &strSrc, xMY_PCSTR pRSrc, std::size_t uRLen){
			xInit();
			xJoinConstruct(pLSrc, (pLSrc != nullptr) ? xMy_len(pLSrc) : 0, strSrc, pRSrc, uRLen);
		}
		GenericString(xMY_PCSTR pLSrc, std::size_t uLLen, const GenericString &strSrc, xMY_PCSTR pRSrc, std::size_t uRLen){
			xInit();
			xJoinConstruct(pLSrc, uLLen, strSrc, pRSrc, uRLen);
		}
		GenericString(xMY_PCSTR pLSrc, GenericString &&strSrc, xMY_PCSTR pRSrc){
			xInit();
			xJoinConstruct(pLSrc, (pLSrc != nullptr) ? xMy_len(pLSrc) : 0, std::move(strSrc), pRSrc, (pRSrc != nullptr) ? xMy_len(pRSrc) : 0);
		}
		GenericString(xMY_PCSTR pLSrc, std::size_t uLLen, GenericString &&strSrc, xMY_PCSTR pRSrc){
			xInit();
			xJoinConstruct(pLSrc, uLLen, std::move(strSrc), pRSrc, (pRSrc != nullptr) ? xMy_len(pRSrc) : 0);
		}
		GenericString(xMY_PCSTR pLSrc, GenericString &&strSrc, xMY_PCSTR pRSrc, std::size_t uRLen){
			xInit();
			xJoinConstruct(pLSrc, (pLSrc != nullptr) ? xMy_len(pLSrc) : 0, std::move(strSrc), pRSrc, uRLen);
		}
		GenericString(xMY_PCSTR pLSrc, std::size_t uLLen, GenericString &&strSrc, xMY_PCSTR pRSrc, std::size_t uRLen){
			xInit();
			xJoinConstruct(pLSrc, uLLen, std::move(strSrc), pRSrc, uRLen);
		}

		GenericString &operator=(xMY_PCSTR rhs){
			Assign(rhs);
			return *this;
		}
		GenericString &operator=(xMY_CHAR rhs){
			const xMY_PSTR pBuff = GetBuffer();
			pBuff[0] = rhs;
			pBuff[1] = 0;
			xm_uCachedLen = 1;
			return *this;
		}
		GenericString &operator=(const GenericString &rhs){
			if(&rhs != this){
				Assign(rhs.GetBuffer(), rhs.GetLength());
			}
			return *this;
		}
		GenericString &operator=(GenericString &&rhs){
			if(&rhs != this){
				if(rhs.xm_uExBuffCap == 0){
					Assign(rhs.xm_InBuff, rhs.GetLength());
				} else {
					std::swap(xm_ExBuff, rhs.xm_ExBuff);
					std::swap(xm_uExBuffCap, rhs.xm_uExBuffCap);
					std::swap(xm_uCachedLen, rhs.xm_uCachedLen);
				}
			}
			return *this;
		}
		xDECL_DEPRECATED_IMPLICIT_CHARSET_CONVERSION GenericString &operator=(const xGenericStringBase &rhs){
			return ConvFrom(rhs);
		}

		virtual ~GenericString(){
			delete[] xm_ExBuff;
		}
	private:
		void xInit(){
			xm_InBuff[0]	= 0;
			xm_ExBuff		= nullptr;
			xm_uExBuffCap	= 0;
			xm_uCachedLen	= 0;
		}
		void xJoinConstruct(xMY_PCSTR pLSrc, std::size_t uLLen, const GenericString &strSrc, xMY_PCSTR pRSrc, std::size_t uRLen){
			const xMY_PCSTR pMSrc = strSrc.GetBuffer();
			const std::size_t uMLen = strSrc.GetLength();

			const std::size_t uTotalLen = uLLen + uMLen + uRLen;

			Resize(uTotalLen, false);
			const xMY_PSTR pLDst = GetBuffer();
			const xMY_PSTR pMDst = pLDst + uLLen;
			const xMY_PSTR pRDst = pMDst + uMLen;

			// 因为这是在构造函数里，我们可以确定左右填充和当前对象的内存区域一定不会有重叠。
			if(pLSrc != nullptr){
				std::memcpy(pLDst, pLSrc, uLLen * sizeof(xMY_CHAR));
			}
			if(pMSrc != nullptr){
				std::memcpy(pMDst, pMSrc, uMLen * sizeof(xMY_CHAR));
			}
			if(pRSrc != nullptr){
				std::memcpy(pRDst, pRSrc, uRLen * sizeof(xMY_CHAR));
			}
			xm_uCachedLen = uTotalLen;
		}
		void xJoinConstruct(xMY_PCSTR pLSrc, std::size_t uLLen, GenericString &&strSrc, xMY_PCSTR pRSrc, std::size_t uRLen){
			const xMY_PCSTR pMSrc = strSrc.GetBuffer();
			const std::size_t uMLen = strSrc.GetLength();

			const std::size_t uTotalLen = uLLen + uMLen + uRLen;
			if(uTotalLen < strSrc.GetCapacity()){
				// 不用申请新的缓冲区。
				// 现在检测可能存在的内存冲突。
				const xMY_PSTR pLDst = strSrc.GetBuffer();
				const xMY_PSTR pMDst = pLDst + uLLen;
				const xMY_PSTR pRDst = pMDst + uMLen;

				// 用查表法解决有向图环路问题，因为只有三个点 (L, M, R)，查表法最快。
				// 对于某一项，三个数字表示，按照此顺序进行拷贝可以避免内存冲突。
				// 若该项的第一个数字为 -1，说明图中有环，因此没有这样的顺序存在。
				static const int aarnOrderTable[64][3] = {
					{  0, 1, 2 },	{  1, 0, 2 },	{  1, 2, 0 },	{  1, 2, 0 },
					{  0, 1, 2 },	{ -1, 0, 0 },	{  2, 0, 1 },	{ -1, 0, 0 },
					{  0, 2, 1 },	{  2, 1, 0 },	{  2, 0, 1 },	{  2, 1, 0 },
					{  0, 2, 1 },	{ -1, 0, 0 },	{  2, 0, 1 },	{ -1, 0, 0 },
					{  0, 1, 2 },	{  1, 0, 2 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{  0, 1, 2 },	{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{  0, 2, 1 },	{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{  0, 2, 1 },	{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{  0, 1, 2 },	{  1, 0, 2 },	{  1, 2, 0 },	{  1, 2, 0 },
					{  0, 1, 2 },	{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{  0, 1, 2 },	{  1, 0, 2 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{  0, 1, 2 },	{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },
					{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 },	{ -1, 0, 0 }
				};
				const int *const pnMyOrder = aarnOrderTable[
					(xOverlapChk(pMSrc, uMLen, pLDst, uLLen) ?  1 : 0) |
					(xOverlapChk(pRSrc, uRLen, pLDst, uLLen) ?  2 : 0) |
					(xOverlapChk(pLSrc, uLLen, pMDst, uMLen) ?  4 : 0) |
					(xOverlapChk(pRSrc, uRLen, pMDst, uMLen) ?  8 : 0) |
					(xOverlapChk(pLSrc, uLLen, pRDst, uRLen) ? 16 : 0) |
					(xOverlapChk(pMSrc, uMLen, pRDst, uRLen) ? 32 : 0)
				];

				if(pnMyOrder[0] != -1){
					// 如果图中没有环，只需按照依赖顺序拷贝，就可以避免内存冲突。
					for(std::size_t i = 0; i < 3; ++i){
						switch(pnMyOrder[i]){
							case 0:
								if(pLSrc != nullptr){
									std::memmove(pLDst, pLSrc, uLLen * sizeof(xMY_CHAR));
								}
								break;
							case 1:
								if(pMSrc != nullptr){
									std::memmove(pMDst, pMSrc, uMLen * sizeof(xMY_CHAR));
								}
								break;
							case 2:
								if(pRSrc != nullptr){
									std::memmove(pRDst, pRSrc, uRLen * sizeof(xMY_CHAR));
								}
								break;
						}
					}
					pLDst[uTotalLen] = 0;
					strSrc.xm_uCachedLen = uTotalLen;

					*this = std::move(strSrc);
					return;
				}
				// 如果图中有环，按照左值引用函数内同样的方法处理。
			}

			xJoinConstruct(pLSrc, uLLen, (const GenericString &)strSrc, pRSrc, uRLen);
		}

		// 增加缓冲区大小至至少 uMinCap 个字符（不包含末尾的结束符）；且若 bKeepContent = true，保持原有字符串不变。
		void xReserve(std::size_t uMinCap, bool bKeepContent){
			// 分为四种情况：
			// 1. 原先使用内部缓冲区，新的缓冲区大小小于或等于内部缓冲区大小：	不做任何事情。
			// 2. 原先使用内部缓冲区，新的缓冲区大小大于内部缓冲区大小：		创建外部缓冲区，将内部缓冲区复制到外部。
			// 3. 原先使用外部缓冲区，新的缓冲区大小小于或等于内部缓冲区大小：	不做任何事情。
			// 4. 原先使用外部缓冲区，新的缓冲区大小大于内部缓冲区大小：		扩大外部缓冲区。

			// http://www-graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
			//   Round up to the next highest power of 2
			uMinCap |= uMinCap >> 1;
			uMinCap |= uMinCap >> 2;
			uMinCap |= uMinCap >> 4;
			uMinCap |= uMinCap >> 8;
			uMinCap |= uMinCap >> 16;
			++uMinCap;

			if(uMinCap > COUNTOF(xm_InBuff)){
				if(xm_uExBuffCap == 0){
					// 情况 2。
					xm_uExBuffCap = uMinCap;
					xm_ExBuff = new xMY_CHAR[xm_uExBuffCap];
					if(bKeepContent){
						std::memcpy(xm_ExBuff, xm_InBuff, COUNTOF(xm_InBuff) * sizeof(xMY_CHAR));
					}
				} else if(xm_uExBuffCap < uMinCap){
					// 情况 4。
					const xMY_PCSTR pOldExBuff = xm_ExBuff;
					xm_uExBuffCap = uMinCap;
					xm_ExBuff = new xMY_CHAR[xm_uExBuffCap];
					if(bKeepContent){
						std::memcpy(xm_ExBuff, pOldExBuff, (xMy_len(pOldExBuff) + 1) * sizeof(xMY_CHAR));
					}
					delete[] pOldExBuff;
				}
			}
			// 情况 1、3。
		}
	public:
		xMY_PCSTR GetBuffer() const {
			return (xm_uExBuffCap == 0) ? xm_InBuff : xm_ExBuff;
		}
		xMY_PSTR GetBuffer(){
			xm_uCachedLen = (std::size_t)-1;
			return (xm_uExBuffCap == 0) ? xm_InBuff : xm_ExBuff;
		}
		std::size_t GetLength() const {
			if(xm_uCachedLen == (std::size_t)-1){
				xm_uCachedLen = xMy_len(GetBuffer());
			}
			return xm_uCachedLen;
		}
		std::size_t GetCapacity() const {
			return ((xm_uExBuffCap == 0) ? COUNTOF(xm_InBuff) : xm_uExBuffCap) - 1;
		}

		GenericString &Clear(){
			GetBuffer()[0] = 0;
			xm_uCachedLen = 0;
			return *this;
		}
		bool IsEmpty() const {
			return GetBuffer()[0] == 0;
		}

		GenericString &Assign(xMY_PCSTR pSrc){
			return Assign(pSrc, (pSrc != nullptr) ? xMy_len(pSrc) : 0);
		}
		GenericString &Assign(xMY_PCSTR pSrc, std::size_t uLen){
			if(uLen == 0){
				Clear();
			} else {
				const xMY_PSTR pOldBuffer = GetBuffer();
				if(xOverlapChk(pOldBuffer, GetCapacity(), pSrc, uLen)){
					xReserve(uLen, true);
					const xMY_PSTR pBuffer = GetBuffer();
					std::memmove(pBuffer, pBuffer + (pSrc - pOldBuffer), uLen * sizeof(xMY_CHAR));
					pBuffer[uLen] = 0;
				} else {
					xReserve(uLen, false);
					const xMY_PSTR pBuffer = GetBuffer();
					std::memcpy(pBuffer, pSrc, uLen * sizeof(xMY_CHAR));
					pBuffer[uLen] = 0;
				}
				xm_uCachedLen = uLen;
			}
			return *this;
		}
		GenericString &Assign(xMY_PCSTR pBegin, xMY_PCSTR pEnd){
			return Assign(pBegin, (std::size_t)(pEnd - pBegin));
		}
		GenericString &Resize(std::size_t uMinLen, bool bKeepContent = true){
			// 这个函数将缓冲区大小调整到至少 (uMinLen + 1) 个字符，并且将第 uMinLen 个字符填零。
			// 如果 bKeepContent = true，原有的字符串结束符以及之前的内容保持不变，但原先字符串结束符和新填充的字符串结束符之间的内容是未定义的；
			// 否则，缓冲区中除了上面提及的零以外的任何内容都未定义。

			xReserve(uMinLen, bKeepContent);
			GetBuffer()[uMinLen] = 0;
			xm_uCachedLen = 0;
			return *this;
		}

		void Push(xMY_CHAR ch){
			const std::size_t uCurrentLen = GetLength();

			xReserve(uCurrentLen + 1, true);
			const xMY_PSTR pBuffer = GetBuffer();

			pBuffer[uCurrentLen]		= ch;
			pBuffer[uCurrentLen + 1]	= 0;

			xm_uCachedLen = uCurrentLen + 1;
		}
		xMY_CHAR Pop(){
			const std::size_t uCurrentLen = GetLength();
			if(uCurrentLen == 0){
				return 0;
			}

			const xMY_PSTR pBuffer = GetBuffer();
			const xMY_CHAR chRet = pBuffer[uCurrentLen - 1];
			pBuffer[uCurrentLen - 1]	= 0;

			xm_uCachedLen = uCurrentLen - 1;

			return chRet;
		}

		GenericString &Append(xMY_PCSTR pSrc){
			return Append(pSrc, (pSrc != nullptr) ? xMy_len(pSrc) : 0);
		}
		GenericString &Append(xMY_PCSTR pSrc, std::size_t uLen){
			if((pSrc != nullptr) && (uLen != 0)){
				const std::size_t uCurrentLen = GetLength();
				const std::size_t uNewLen = uCurrentLen + uLen;

				const xMY_PSTR pOldBuffer = GetBuffer();
				if(xOverlapChk(pOldBuffer, GetCapacity(), pSrc, uLen)){
					xReserve(uNewLen, true);
					const xMY_PSTR pBuffer = GetBuffer();
					std::memmove(pBuffer + uCurrentLen, pBuffer + (pSrc - pOldBuffer), uLen * sizeof(xMY_CHAR));
					pBuffer[uNewLen] = 0;
				} else {
					xReserve(uNewLen, true);
					const xMY_PSTR pBuffer = GetBuffer();
					std::memcpy(pBuffer + uCurrentLen, pSrc, uLen * sizeof(xMY_CHAR));
					pBuffer[uNewLen] = 0;
				}
				xm_uCachedLen = uNewLen;
			}
			return *this;
		}
		GenericString &Append(xMY_CHAR ch){
			Push(ch);
			return *this;
		}

		int Compare(xMY_PCSTR rhs) const {
			return xMy_cmp(GetBuffer(), rhs);
		}

		GenericString &TrimLeft(){
			const std::size_t uLen = GetLength();
			const xMY_PSTR pBegin = GetBuffer();
			const xMY_PSTR pEnd = pBegin + uLen;

			xMY_PSTR pCur;
			for(pCur = pBegin; pCur < pEnd; ++pCur){
				if(!xMy_isspace(*pCur)){
					break;
				}
			}
			if(pCur > pBegin){
				std::memmove(pBegin, pCur, (pEnd - pCur + 1) * sizeof(xMY_CHAR));
			}
			xm_uCachedLen = (std::size_t)(pEnd - pCur);

			return *this;
		}
		GenericString &TrimRight(){
			const std::size_t uLen = GetLength();
			const xMY_PSTR pBegin = GetBuffer();
			const xMY_PSTR pEnd = pBegin + uLen;
			xMY_PSTR pCur;

			for(pCur = pEnd; pCur > pBegin; --pCur){
				if(!xMy_isspace(*(pCur - 1))){
					break;
				}
			}
			if(pCur < pEnd){
				*pCur = 0;
			}
			xm_uCachedLen = (std::size_t)(pCur - pBegin);

			return *this;
		}
		GenericString &Trim(){
			return TrimRight().TrimLeft();
		}

		std::ptrdiff_t IndexOf(xMY_PCSTR pWhat, std::ptrdiff_t nOffset = 0) const {
			return IndexOf(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, nOffset);
		}
		std::ptrdiff_t IndexOf(xMY_PCSTR pWhat, std::size_t uWhatLen, std::ptrdiff_t nOffset = 0) const {
			if(uWhatLen == 0){
				return NOT_FOUND;
			}

			const std::size_t uLen = GetLength();
			const xMY_PCSTR pBegin = GetBuffer();
			const xMY_PCSTR pEnd = pBegin + uLen;

			xMY_PCSTR pCur = (nOffset >= 0) ? (pBegin + nOffset) : (pEnd + nOffset);
			if((pCur < pBegin) || (pCur > pEnd)){
				// 越界。
				return NOT_FOUND;
			}
			while(pCur < pEnd - uWhatLen + 1){
				if((*pCur == *pWhat) && (xMy_ncmp(pCur, pWhat, uWhatLen) == 0)){
					return pCur - pBegin;
				}
				++pCur;
			}
			return NOT_FOUND;
		}
		std::ptrdiff_t IndexOf(xMY_CHAR ch, std::ptrdiff_t nOffset = 0) const {
			if(ch == 0){
				return NOT_FOUND;
			}

			const std::size_t uLen = GetLength();
			const xMY_PCSTR pBegin = GetBuffer();
			const xMY_PCSTR pEnd = pBegin + uLen;

			xMY_PCSTR pCur = (nOffset >= 0) ? (pBegin + nOffset) : (pEnd + nOffset);
			if((pCur < pBegin) || (pCur > pEnd)){
				// 越界。
				return NOT_FOUND;
			}
			while(pCur < pEnd){
				if(*pCur == ch){
					return pCur - pBegin;
				}
				++pCur;
			}
			return NOT_FOUND;
		}
		std::ptrdiff_t LastIndexOf(xMY_PCSTR pWhat, std::ptrdiff_t nOffset = 0) const {
			return LastIndexOf(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, nOffset);
		}
		std::ptrdiff_t LastIndexOf(xMY_PCSTR pWhat, std::size_t uWhatLen, std::ptrdiff_t nOffset = -1) const {
			if(uWhatLen == 0){
				return NOT_FOUND;
			}

			const std::size_t uLen = GetLength();
			const xMY_PCSTR pBegin = GetBuffer();
			const xMY_PCSTR pEnd = pBegin + uLen;

			xMY_PCSTR pCur = (nOffset >= 0) ? (pBegin + nOffset) : (pEnd + nOffset);
			if((pCur < pBegin) || (pCur > pEnd)){
				// 越界。
				return NOT_FOUND;
			}
			pCur -= uWhatLen - 1;
			while(pCur >= pBegin){
				if((*pCur == *pWhat) && (xMy_ncmp(pCur, pWhat, uWhatLen) == 0)){
					return pCur - pBegin;
				}
				--pCur;
			}
			return NOT_FOUND;
		}
		std::ptrdiff_t LastIndexOf(xMY_CHAR ch, std::ptrdiff_t nOffset = -1) const {
			if(ch == 0){
				return NOT_FOUND;
			}
			const std::size_t uLen = GetLength();
			const xMY_PCSTR pBegin = GetBuffer();
			const xMY_PCSTR pEnd = pBegin + uLen;

			xMY_PCSTR pCur = (nOffset >= 0) ? (pBegin + nOffset) : (pEnd + nOffset);
			if((pCur < pBegin) || (pCur > pEnd)){
				// 越界。
				return NOT_FOUND;
			}
			while(pCur >= pBegin){
				if(*pCur == ch){
					return pCur - pBegin;
				}
				--pCur;
			}
			return NOT_FOUND;
		}

		GenericString Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = 0) const {
			// 获得字符串中一个区间上的片段。
			// nEnd 为零时是说字符串最后一个字符的后一个字符，这点和其它地方不一样。

			const std::size_t uLen = GetLength();
			const xMY_PCSTR pBegin = GetBuffer();
			const xMY_PCSTR pEnd = pBegin + uLen;

			xMY_PCSTR pSubBegin = (nBegin >= 0) ? (pBegin + nBegin) : (pEnd + nBegin);
			xMY_PCSTR pSubEnd = (nEnd > 0) ? (pBegin + nEnd) : (pEnd + nEnd);
			if((pSubEnd <= pSubBegin) || (pSubBegin >= pEnd) || (pSubEnd <= pBegin)){
				return GenericString();
			}
			if(pSubBegin < pBegin){
				pSubBegin = pBegin;
			}
			if(pSubEnd > pEnd){
				pSubEnd = pEnd;
			}
			return GenericString(pSubBegin, (std::size_t)(pSubEnd - pSubBegin));
		}
		GenericString SubStr(std::ptrdiff_t nOffset, std::ptrdiff_t nSubLen) const {
			if(nSubLen > 0){
				if((nOffset < 0) && (nOffset + nSubLen > 0)){
					return Slice(nOffset, 0);
				} else {
					return Slice(nOffset, nOffset + nSubLen);
				}
			} else if(nSubLen < 0){
				if((nOffset > 0) && (nOffset + nSubLen + 1< 0)){
					return Slice(0, nOffset + 1);
				} else {
					return Slice(nOffset + nSubLen + 1, nOffset + 1);
				}
			} else {
				return GenericString();
			}
		}

		std::ptrdiff_t Replace(std::ptrdiff_t nOffset, std::ptrdiff_t nSubLen, xMY_PCSTR pReplacement){
			return Replace(nOffset, nSubLen, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0);
		}
		std::ptrdiff_t Replace(std::ptrdiff_t nOffset, std::ptrdiff_t nSubLen, xMY_PCSTR pReplacement, std::size_t uReplacementLen){
			const std::size_t uLen = GetLength();
			const xMY_PSTR pBegin = GetBuffer();
			const xMY_PSTR pEnd = pBegin + uLen;

			xMY_PSTR pSubBegin = (nOffset >= 0) ? (pBegin + nOffset) : (pEnd + nOffset);
			if(nSubLen < 0){
				pSubBegin += nSubLen + 1;
				nSubLen = -nSubLen;
			}
			xMY_PSTR pSubEnd = pSubBegin + nSubLen;

			if((pSubBegin >= pEnd) || (pSubEnd < pBegin)){
				// 越界。
				return NOT_FOUND;
			}
			if(pSubBegin < pBegin){
				pSubBegin = pBegin;
			}
			if(pSubEnd >= pEnd){
				pSubEnd = pEnd;
			}

			const std::size_t uLLen = (std::size_t)(pSubBegin - pBegin);
			const std::size_t uCutOffLen = (std::size_t)(pSubEnd - pSubBegin);
			const std::size_t uRLen = (std::size_t)(pEnd - pSubEnd);
			const std::size_t uNewTotalLen = uLLen + uReplacementLen + uRLen;

			xReserve(uNewTotalLen, true);
			const xMY_PSTR pNewBuffer = GetBuffer();
			const xMY_PSTR pMDst = pNewBuffer + uLLen;
			const xMY_PSTR pRDst = pMDst + uReplacementLen;

			if(uReplacementLen < uCutOffLen){
				// 填入的字符串比原来的要短。
				//    llll mmmm rrrrrrr
				// -> llll nn rrrrrrr
				// 复制顺序先中后右。
				if(pReplacement != nullptr){
					std::memmove(pMDst, pReplacement, uReplacementLen * sizeof(xMY_CHAR));
				}
				std::memmove(pRDst, pNewBuffer + (std::ptrdiff_t)(uLLen + uCutOffLen), uRLen * sizeof(xMY_CHAR));
			} else if(uReplacementLen > uCutOffLen){
				// 填入的字符串比原来的要长。
				//    llll mmmm rrrrrrr
				// -> llll nnnnnnnn rrrrrrr
				// 复制顺序先右后中。
				std::memmove(pRDst, pNewBuffer + (std::ptrdiff_t)(uLLen + uCutOffLen), uRLen * sizeof(xMY_CHAR));
				if(pReplacement != nullptr){
					std::memmove(pMDst, pReplacement, uReplacementLen * sizeof(xMY_CHAR));
				}
			} else {
				// 填入的字符串和原来的一样长。
				//    llll mmmm rrrrrrr
				// -> llll nnnn rrrrrrr
				// 复制中间即可。
				std::memmove(pMDst, pReplacement, uReplacementLen * sizeof(xMY_CHAR));
			}
			pNewBuffer[uNewTotalLen] = 0;

			return pRDst - pNewBuffer;
		}

		std::ptrdiff_t Replace(xMY_PCSTR pWhat, xMY_PCSTR pReplacement, std::ptrdiff_t nOffset = 0){
			return Replace(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0, nOffset);
		}
		std::ptrdiff_t Replace(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement, std::ptrdiff_t nOffset = 0){
			return Replace(pWhat, uWhatLen, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0, nOffset);
		}
		std::ptrdiff_t Replace(xMY_PCSTR pWhat, xMY_PCSTR pReplacement, std::size_t uReplacementLen, std::ptrdiff_t nOffset = 0){
			return Replace(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, uReplacementLen, nOffset);
		}
		std::ptrdiff_t Replace(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement, std::size_t uReplacementLen, std::ptrdiff_t nOffset = 0){
			const std::ptrdiff_t nPos = IndexOf(pWhat, uWhatLen, nOffset);
			if(nPos == NOT_FOUND){
				return NOT_FOUND;
			}
			return Replace(nPos, (std::ptrdiff_t)uWhatLen, pReplacement, uReplacementLen);
		}
		std::ptrdiff_t LastReplace(xMY_PCSTR pWhat, xMY_PCSTR pReplacement, std::ptrdiff_t nOffset = 0){
			return LastReplace(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0, nOffset);
		}
		std::ptrdiff_t LastReplace(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement, std::ptrdiff_t nOffset = 0){
			return LastReplace(pWhat, uWhatLen, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0, nOffset);
		}
		std::ptrdiff_t LastReplace(xMY_PCSTR pWhat, xMY_PCSTR pReplacement, std::size_t uReplacementLen, std::ptrdiff_t nOffset = 0){
			return LastReplace(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, uReplacementLen, nOffset);
		}
		std::ptrdiff_t LastReplace(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement, std::size_t uReplacementLen, std::ptrdiff_t nOffset = -1){
			const std::ptrdiff_t nPos = LastIndexOf(pWhat, uWhatLen, nOffset);
			if(nPos == NOT_FOUND){
				return NOT_FOUND;
			}
			return Replace(nPos, (std::ptrdiff_t)uWhatLen, pReplacement, uReplacementLen);
		}
		std::size_t ReplaceAllAfter(xMY_PCSTR pWhat, xMY_PCSTR pReplacement, std::ptrdiff_t nOffset = 0){
			return ReplaceAllAfter(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0, nOffset);
		}
		std::size_t ReplaceAllAfter(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement, std::ptrdiff_t nOffset = 0){
			return ReplaceAllAfter(pWhat, uWhatLen, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0, nOffset);
		}
		std::size_t ReplaceAllAfter(xMY_PCSTR pWhat, xMY_PCSTR pReplacement, std::size_t uReplacementLen, std::ptrdiff_t nOffset = 0){
			return ReplaceAllAfter(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, uReplacementLen, nOffset);
		}
		std::size_t ReplaceAllAfter(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement, std::size_t uReplacementLen, std::ptrdiff_t nOffset = 0){
			std::size_t uCount = 0;
			std::ptrdiff_t nCur = nOffset;
			for(;;){
				nCur = Replace(pWhat, uWhatLen, pReplacement, uReplacementLen, nCur);
				if(nCur == NOT_FOUND){
					break;
				} else {
					++uCount;
				}
			}
			return uCount;
		}
		std::size_t ReplaceAllBefore(xMY_PCSTR pWhat, xMY_PCSTR pReplacement, std::ptrdiff_t nOffset = 0){
			return ReplaceAllBefore(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0, nOffset);
		}
		std::size_t ReplaceAllBefore(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement, std::ptrdiff_t nOffset = 0){
			return ReplaceAllBefore(pWhat, uWhatLen, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0, nOffset);
		}
		std::size_t ReplaceAllBefore(xMY_PCSTR pWhat, xMY_PCSTR pReplacement, std::size_t uReplacementLen, std::ptrdiff_t nOffset = 0){
			return ReplaceAllBefore(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, uReplacementLen, nOffset);
		}
		std::size_t ReplaceAllBefore(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement, std::size_t uReplacementLen, std::ptrdiff_t nOffset = 0){
			std::size_t uCount = 0;
			std::ptrdiff_t nCur = nOffset;
			for(;;){
				nCur = LastReplace(pWhat, uWhatLen, pReplacement, uReplacementLen, nCur);
				if(nCur == NOT_FOUND){
					break;
				} else {
					++uCount;
				}
				nCur -= (std::ptrdiff_t)uReplacementLen;
			}
			return uCount;
		}
		std::size_t ReplaceAll(xMY_PCSTR pWhat, xMY_PCSTR pReplacement){
			return ReplaceAll(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0);
		}
		std::size_t ReplaceAll(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement){
			return ReplaceAll(pWhat, uWhatLen, pReplacement, (pReplacement != nullptr) ? xMy_len(pReplacement) : 0);
		}
		std::size_t ReplaceAll(xMY_PCSTR pWhat, xMY_PCSTR pReplacement, std::size_t uReplacementLen){
			return ReplaceAll(pWhat, (pWhat != nullptr) ? xMy_len(pWhat) : 0, pReplacement, uReplacementLen);
		}
		std::size_t ReplaceAll(xMY_PCSTR pWhat, std::size_t uWhatLen, xMY_PCSTR pReplacement, std::size_t uReplacementLen){
			return ReplaceAllAfter(pWhat, uWhatLen, pReplacement, uReplacementLen, 0);
		}

		GenericString &Format(xMY_PCSTR pszFormat, ...){
			std::va_list ArgList;
			va_start(ArgList, pszFormat);
			VFormat(pszFormat, ArgList);
			va_end(ArgList);
			return *this;
		}
		GenericString &VFormat(xMY_PCSTR pszFormat, std::va_list ArgList){
			const int nFmtLen = xMy_vsnprintf(nullptr, 0, pszFormat, ArgList);
			if(nFmtLen > 0){
				xReserve((std::size_t)nFmtLen, false);
				xMy_vsnprintf(GetBuffer(), (std::size_t)nFmtLen + 1, pszFormat, ArgList);
			}
			return *this;
		}

		GenericString &ConvFrom(const xGenericStringBase &src){
			return *this = xMy_Deunify(src.xMy_Unify());
		}
		GenericString &ConvFrom(const GenericString &src){
			return *this = src;
		}
		GenericString &ConvFrom(GenericString &&src){
			return *this = std::move(src);
		}
	public:
		operator xMY_PCSTR() const {
			return GetBuffer();
		}
		operator xMY_PSTR(){
			return GetBuffer();
		}

		operator bool() const {
			return !IsEmpty();
		}

		GenericString &operator+=(const GenericString &rhs){
			return Append(rhs.GetBuffer(), rhs.GetLength());
		}
		GenericString &operator+=(GenericString &&rhs){
			if(GetCapacity() >= rhs.GetCapacity()){
				return Append(rhs.GetBuffer(), rhs.GetLength());
			} else {
				return *this = std::move(GenericString(GetBuffer(), GetLength(), std::move(rhs), nullptr, 0));
			}
		}
		GenericString &operator+=(xMY_PCSTR rhs){
			return Append(rhs);
		}
		GenericString &operator+=(xMY_CHAR rhs){
			return Append(rhs);
		}
	};

	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(const GenericString<xMY_CHAR, ENCODING> &lhs, const GenericString<xMY_CHAR, ENCODING> &rhs){
		return GenericString<xMY_CHAR, ENCODING>(nullptr, lhs, rhs);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(GenericString<xMY_CHAR, ENCODING> &&lhs, const GenericString<xMY_CHAR, ENCODING> &rhs){
		return GenericString<xMY_CHAR, ENCODING>(nullptr, std::move(lhs), rhs);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(const GenericString<xMY_CHAR, ENCODING> &lhs, GenericString<xMY_CHAR, ENCODING> &&rhs){
		return GenericString<xMY_CHAR, ENCODING>(lhs, std::move(rhs), nullptr);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(GenericString<xMY_CHAR, ENCODING> &&lhs, GenericString<xMY_CHAR, ENCODING> &&rhs){
		if(lhs.GetCapacity() >= rhs.GetCapacity()){
			return GenericString<xMY_CHAR, ENCODING>(nullptr, std::move(lhs), rhs);
		} else {
			return GenericString<xMY_CHAR, ENCODING>(lhs, std::move(rhs), nullptr);
		}
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(const xMY_CHAR *lhs, const GenericString<xMY_CHAR, ENCODING> &rhs){
		return GenericString<xMY_CHAR, ENCODING>(lhs, rhs, nullptr);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(xMY_CHAR *lhs, const GenericString<xMY_CHAR, ENCODING> &rhs){
		return GenericString<xMY_CHAR, ENCODING>(lhs, rhs, nullptr);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(xMY_CHAR lhs, const GenericString<xMY_CHAR, ENCODING> &rhs){
		return GenericString<xMY_CHAR, ENCODING>(&lhs, 1, rhs, nullptr);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(const xMY_CHAR *lhs, GenericString<xMY_CHAR, ENCODING> &&rhs){
		return GenericString<xMY_CHAR, ENCODING>(lhs, std::move(rhs), nullptr);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(xMY_CHAR *lhs, GenericString<xMY_CHAR, ENCODING> &&rhs){
		return GenericString<xMY_CHAR, ENCODING>(lhs, std::move(rhs), nullptr);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(xMY_CHAR lhs, GenericString<xMY_CHAR, ENCODING> &&rhs){
		return GenericString<xMY_CHAR, ENCODING>(&lhs, 1, std::move(rhs), nullptr);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(const GenericString<xMY_CHAR, ENCODING> &lhs, const xMY_CHAR *rhs){
		return GenericString<xMY_CHAR, ENCODING>(nullptr, lhs, rhs);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(const GenericString<xMY_CHAR, ENCODING> &lhs, xMY_CHAR *rhs){
		return GenericString<xMY_CHAR, ENCODING>(nullptr, lhs, rhs);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(const GenericString<xMY_CHAR, ENCODING> &lhs, xMY_CHAR rhs){
		return GenericString<xMY_CHAR, ENCODING>(nullptr, lhs, &rhs, 1);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(GenericString<xMY_CHAR, ENCODING> &&lhs, const xMY_CHAR *rhs){
		return GenericString<xMY_CHAR, ENCODING>(nullptr, std::move(lhs), rhs);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(GenericString<xMY_CHAR, ENCODING> &&lhs, xMY_CHAR *rhs){
		return GenericString<xMY_CHAR, ENCODING>(nullptr, std::move(lhs), rhs);
	}
	template<typename xMY_CHAR, STR_ENCODING ENCODING>
	GenericString<xMY_CHAR, ENCODING> operator+(GenericString<xMY_CHAR, ENCODING> &&lhs, xMY_CHAR rhs){
		return GenericString<xMY_CHAR, ENCODING>(nullptr, std::move(lhs), &rhs, 1);
	}

	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator==(const GenericString<CHAR_TYPE, ENCODING> &lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return lhs.Compare((const CHAR_TYPE *)rhs) == 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator==(const GenericString<CHAR_TYPE, ENCODING> &lhs, const CHAR_TYPE *rhs){
		return lhs.Compare(rhs) == 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator==(const GenericString<CHAR_TYPE, ENCODING> &lhs, CHAR_TYPE *rhs){
		return lhs.Compare(rhs) == 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator==(const CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) == 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator==(CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) == 0;
	}

	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator!=(const GenericString<CHAR_TYPE, ENCODING> &lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return lhs.Compare((const CHAR_TYPE *)rhs) != 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator!=(const GenericString<CHAR_TYPE, ENCODING> &lhs, const CHAR_TYPE *rhs){
		return lhs.Compare(rhs) != 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator!=(const GenericString<CHAR_TYPE, ENCODING> &lhs, CHAR_TYPE *rhs){
		return lhs.Compare(rhs) != 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator!=(const CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) != 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator!=(CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) != 0;
	}

	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<(const GenericString<CHAR_TYPE, ENCODING> &lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return lhs.Compare((const CHAR_TYPE *)rhs) < 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<(const GenericString<CHAR_TYPE, ENCODING> &lhs, const CHAR_TYPE *rhs){
		return lhs.Compare(rhs) < 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<(const GenericString<CHAR_TYPE, ENCODING> &lhs, CHAR_TYPE *rhs){
		return lhs.Compare(rhs) < 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<(const CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) >= 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<(CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) >= 0;
	}

	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>(const GenericString<CHAR_TYPE, ENCODING> &lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return lhs.Compare((const CHAR_TYPE *)rhs) > 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>(const GenericString<CHAR_TYPE, ENCODING> &lhs, const CHAR_TYPE *rhs){
		return lhs.Compare(rhs) > 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>(const GenericString<CHAR_TYPE, ENCODING> &lhs, CHAR_TYPE *rhs){
		return lhs.Compare(rhs) > 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>(const CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) <= 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>(CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) <= 0;
	}

	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<=(const GenericString<CHAR_TYPE, ENCODING> &lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return lhs.Compare((const CHAR_TYPE *)rhs) <= 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<=(const GenericString<CHAR_TYPE, ENCODING> &lhs, const CHAR_TYPE *rhs){
		return lhs.Compare(rhs) <= 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<=(const GenericString<CHAR_TYPE, ENCODING> &lhs, CHAR_TYPE *rhs){
		return lhs.Compare(rhs) <= 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<=(const CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) > 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator<=(CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) > 0;
	}

	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>=(const GenericString<CHAR_TYPE, ENCODING> &lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return lhs.Compare((const CHAR_TYPE *)rhs) >= 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>=(const GenericString<CHAR_TYPE, ENCODING> &lhs, const CHAR_TYPE *rhs){
		return lhs.Compare(rhs) >= 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>=(const GenericString<CHAR_TYPE, ENCODING> &lhs, CHAR_TYPE *rhs){
		return lhs.Compare(rhs) >= 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>=(const CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) < 0;
	}
	template<typename CHAR_TYPE, STR_ENCODING ENCODING>
	bool operator>=(CHAR_TYPE *lhs, const GenericString<CHAR_TYPE, ENCODING> &rhs){
		return rhs.Compare(lhs) < 0;
	}

	// 内置常用字符串类型定义。
	typedef GenericString<char, ENC_ANSI>		MBString;
	typedef GenericString<wchar_t, ENC_UCS2LE>	WCString;
	typedef GenericString<char, ENC_UTF8>		UTF8String;

	template<>
	inline __attribute__((always_inline)) MBString::xUniString MBString::xMy_Unify() const {
		xUniString ret;

		const char *const pSrcBuff = GetBuffer();
		const int nSrcLen = (int)GetLength();

		const int nDstLen = ::MultiByteToWideChar(CP_ACP, 0, pSrcBuff, nSrcLen, nullptr, 0);
		if(nDstLen > 0){
			ret.resize((std::size_t)nDstLen);
			::MultiByteToWideChar(CP_ACP, 0, pSrcBuff, nSrcLen, &ret[0], nDstLen);
		}
		return std::move(ret);
	}
	template<>
	inline __attribute__((always_inline)) MBString MBString::xMy_Deunify(const MBString::xUniString &src){
		MBString ret;

		const wchar_t *const pSrcBuff = src.data();
		const int nSrcLen = (int)src.size();

		const int nDstLen = ::WideCharToMultiByte(CP_ACP, 0, pSrcBuff, nSrcLen, nullptr, 0, nullptr, nullptr);
		if(nDstLen > 0){
			ret.Resize((std::size_t)nDstLen, false);
			::WideCharToMultiByte(CP_ACP, 0, pSrcBuff, nSrcLen, ret.GetBuffer(), nDstLen, nullptr, nullptr);
		}
		return std::move(ret);
	}
	template<>
	inline __attribute__((always_inline)) std::size_t MBString::xMy_len(const char *str){
		return std::strlen(str);
	}
	template<>
	inline __attribute__((always_inline)) bool MBString::xMy_isspace(char ch){
		return (ch == ' ') || (ch == '\t');
	}
	template<>
	inline __attribute__((always_inline)) int MBString::xMy_cmp(const char *str1, const char *str2){
		return std::strcmp(str1, str2);
	}
	template<>
	inline __attribute__((always_inline)) int MBString::xMy_ncmp(const char *str1, const char *str2, std::size_t cnt){
		return std::strncmp(str1, str2, cnt);
	}
	template<>
	inline __attribute__((always_inline)) int MBString::xMy_vsnprintf(char *dst, std::size_t cap, const char *fmt, std::va_list ArgList){
		return std::vsnprintf(dst, cap, fmt, ArgList);
	}

	template<>
	inline __attribute__((always_inline)) WCString::xUniString WCString::xMy_Unify() const {
		return std::move(xUniString(GetBuffer()));
	}
	template<>
	inline __attribute__((always_inline)) WCString WCString::xMy_Deunify(const WCString::xUniString &src){
		return std::move(WCString(src.data(), src.size()));
	}
	template<>
	inline __attribute__((always_inline)) std::size_t WCString::xMy_len(const wchar_t *str){
		return std::wcslen(str);
	}
	template<>
	inline __attribute__((always_inline)) bool WCString::xMy_isspace(wchar_t ch){
		return (ch == L' ') || (ch == L'\t');
	}
	template<>
	inline __attribute__((always_inline)) int WCString::xMy_cmp(const wchar_t *str1, const wchar_t *str2){
		return std::wcscmp(str1, str2);
	}
	template<>
	inline __attribute__((always_inline)) int WCString::xMy_ncmp(const wchar_t *str1, const wchar_t *str2, std::size_t cnt){
		return std::wcsncmp(str1, str2, cnt);
	}
	template<>
	inline __attribute__((always_inline)) int WCString::xMy_vsnprintf(wchar_t *dst, std::size_t cap, const wchar_t *fmt, std::va_list ArgList){
		return std::vswprintf(dst, cap, fmt, ArgList);
	}

	template<>
	inline __attribute__((always_inline)) UTF8String::xUniString UTF8String::xMy_Unify() const {
		xUniString ret;

		const char *const pSrcBuff = GetBuffer();
		const int nSrcLen = (int)GetLength();

		const int nDstLen = ::MultiByteToWideChar(CP_UTF8, 0, pSrcBuff, nSrcLen, nullptr, 0);
		if(nDstLen > 0){
			ret.resize((std::size_t)nDstLen);
			::MultiByteToWideChar(CP_UTF8, 0, pSrcBuff, nSrcLen, &ret[0], nDstLen);
		}
		return std::move(ret);
	}
	template<>
	inline __attribute__((always_inline)) UTF8String UTF8String::xMy_Deunify(const UTF8String::xUniString &src){
		UTF8String ret;

		const wchar_t *const pSrcBuff = src.data();
		const int nSrcLen = (int)src.size();

		const int nDstLen = ::WideCharToMultiByte(CP_UTF8, 0, pSrcBuff, nSrcLen, nullptr, 0, nullptr, nullptr);
		if(nDstLen > 0){
			ret.Resize((std::size_t)nDstLen, false);
			::WideCharToMultiByte(CP_UTF8, 0, pSrcBuff, nSrcLen, ret.GetBuffer(), nDstLen, nullptr, nullptr);
		}
		return std::move(ret);
	}
	template<>
	inline __attribute__((always_inline)) std::size_t UTF8String::xMy_len(LPCSTR str){
		return std::strlen(str);
	}
	template<>
	inline __attribute__((always_inline)) bool UTF8String::xMy_isspace(char ch){
		return (ch == ' ') || (ch == '\t');
	}
	template<>
	inline __attribute__((always_inline)) int UTF8String::xMy_cmp(LPCSTR str1, LPCSTR str2){
		return std::strcmp(str1, str2);
	}
	template<>
	inline __attribute__((always_inline)) int UTF8String::xMy_ncmp(const char *str1, const char *str2, std::size_t cnt){
		return std::strncmp(str1, str2, cnt);
	}
	template<>
	inline __attribute__((always_inline)) int UTF8String::xMy_vsnprintf(char *dst, std::size_t cap, const char *fmt, std::va_list ArgList){
		return std::vsnprintf(dst, cap, fmt, ArgList);
	}

	template class GenericString<char, ENC_ANSI>;
	template class GenericString<wchar_t, ENC_UCS2LE>;
	template class GenericString<char, ENC_UTF8>;

#ifdef _UNICODE
	typedef WCString TString;
#else
	typedef MBString TString;
#endif
}

#ifdef xDECL_DEPRECATED_IMPLICIT_CHARSET_CONVERSION
#	undef xDECL_DEPRECATED_IMPLICIT_CHARSET_CONVERSION
#endif

#endif
