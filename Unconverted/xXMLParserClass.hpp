// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_XML_PARSER_CLASS_HPP__
#define __MCF_X_XML_PARSER_CLASS_HPP__

#include "StdMCF.hpp"
#include <string>
#include <forward_list>
#include <map>
#include <vector>
#include "xXMLElementClass.hpp"

namespace MCF {
	class XMLDocumentClass::XMLElementClass::xXMLParserClass : NO_COPY_OR_ASSIGN {
	public:
		typedef enum {
			ERR_NONE,
			ERR_DISSOCIATIVE_CDATA,
			ERR_INVALID_ENTITY,
			ERR_MULTIPLE_ROOT_ELEMENT,
			ERR_DISSOCIATIVE_ELEMENT_CLOSEtag,
			ERR_ELEMENT_CLOSEtag_MISMATCH,
			ERR_MISSING_ELEMENT_CLOSEtag,
			ERR_UNRECOGNIZEDtag
		} ERROR_CODE;
	private:
		typedef struct xtagAttribPos {
			const char *pNameBegin;
			const char *pNameEnd;
			const char *pValBegin;
			const char *pValEnd;
		} xATTRIB_POS;

		typedef struct xtagParseException {
			ERROR_CODE eErrorCode;
			const char *pWhere;
			std::string strErrorInfo;

			xtagParseException(ERROR_CODE xeErrorCode, const char *xpWhere, const std::string &xstrErrorInfo = std::string()) :
				eErrorCode(xeErrorCode),
				pWhere(xpWhere),
				strErrorInfo(xstrErrorInfo)
			{ }
			xtagParseException(ERROR_CODE xeErrorCode, const char *xpWhere, std::string &&xstrErrorInfo) :
				eErrorCode(xeErrorCode),
				pWhere(xpWhere),
				strErrorInfo(std::move(xstrErrorInfo))
			{ }
			xtagParseException(const xtagParseException &src) :
				eErrorCode(src.eErrorCode),
				pWhere(src.pWhere),
				strErrorInfo(src.strErrorInfo)
			{ }
			xtagParseException(xtagParseException &&src) :
				eErrorCode(src.eErrorCode),
				pWhere(src.pWhere),
				strErrorInfo(std::move(src.strErrorInfo))
			{ }
		} xPARSE_ERROR;
	private:
		// 匹配成功返回结束位置的下一个字符，否则返回 nullptr。
		static const char *xTryInstruction(const char *pWhere, const char **ppInstTypeBegin, const char **ppInstTypeEnd, const char **ppInstParamBegin, const char **ppInstParamEnd);
		static const char *xTryDocType(const char *pWhere, const char **ppRootElementKeyBegin, const char **ppRootElementKeyEnd, const char **ppSourceTypeBegin, const char **ppSourceTypeEnd, const char **ppIDBegin, const char **ppIDEnd, const char **ppSourceURIBegin, const char **ppSourceURIEnd, const char **ppSubsetBegin, const char **ppSubsetEnd);
		static const char *xTryCData(const char *pWhere, const char **ppCDataBegin, const char **ppCDataEnd);
		static const char *xTryComment(const char *pWhere, const char **ppCommentBegin, const char **ppCommentEnd);
		static const char *xTryElementOpen(const char *pWhere, const char **ppKeyBegin, const char **ppKeyEnd, std::vector<xATTRIB_POS> *pvecAttributes, bool *pbIsComplete);
		static const char *xTryElementClose(const char *pWhere, const char **ppKeyBegin, const char **ppKeyEnd);
	public:
		static bool CanBeginIDs(char ch);
		static bool IsLegalInIDs(char ch);
		static bool IsBlank(char ch);
	private:
		XMLElementClass *xm_pRoot;
		XMLElementClass *xm_pCurrent;
		std::forward_list<std::string> xm_lstKeyStack;

		ERROR_CODE xm_eErrorCode;
		std::string xm_strErrorInfo;
	public:
		xXMLParserClass();
		~xXMLParserClass();
	private:
		void xInit();

		void xOnInstruction(const char *pInstTypeBegin, const char *pInstTypeEnd, const char *pInstParamBegin, const char *pInstParamEnd);
		void xOnDocType(const char *pRootElementKeyBegin, const char *pRootElementKeyEnd, const char *pSourceTypeBegin, const char *pSourceTypeEnd, const char *pIdentifierBegin, const char *pIdentifierEnd, const char *pSourceURIBegin, const char *pSourceURIEnd, const char *pSubsetBegin, const char *pSubsetEnd);
		void xOnCData(const char *pCDataBegin, const char *pCDataEnd);
		void xOnComment(const char *pCommentBegin, const char *pCommentEnd);
		void xOnElementOpen(const char *pKeyBegin, const char *pKeyEnd, const std::vector<xATTRIB_POS> &vecAttributes);
		void xOnElementClose(const char *pKeyBegin, const char *pKeyEnd);
	public:
		const char *Parse(const char *pszXMLString);

		ERROR_CODE GetErrorCode() const;
		const std::string &GetErrorInfo() const;
		XMLElementClass *GetMovableRootElement();
	};
}

#endif
