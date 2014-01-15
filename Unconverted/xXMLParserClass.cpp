// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "xXMLParserClass.hpp"
using namespace MCF;

// 静态成员函数。
const char *XMLDocumentClass::XMLElementClass::xXMLParserClass::xTryInstruction(const char *pWhere, const char **ppInstTypeBegin, const char **ppInstTypeEnd, const char **ppInstParamBegin, const char **ppInstParamEnd){
	// XML 指令正则表达式			 <\?\w[\w\.\-_:α]*(\s+.*?)?\?>		其中α代表二进制位于范围 0x80-0xFF 内的所有字符，下同。
	//								|| | |           |   | |     ||
	// DFA 状态集合					|| | |           |   | |     ||
	//								|| | |           |   | |     ||
	// INIT						----/| | |           |   | |     ||
	// LT_MATCH					-----/ | |           |   | |     ||
	// QM_MATCH_LEFT			-------/ |           |   | |     ||
	// INST_NAME_BODY			---------/           |   | |     ||
	// (INST_NAME_BODY)			---------------------/   | |     ||
	// INST_NAME_ACCEPTED		-------------------------/ |     ||
	// INST_PARAM_BODY			---------------------------/     ||
	// QM_MATCH_RIGHT			---------------------------------/|
	// (GT_MATCH)				----------------------------------/

	enum {
		INIT,
		LT_MATCH,
		QM_MATCH_LEFT,
		INST_NAME_BODY,
		INST_NAME_ACCEPTED,
		INST_PARAM_BODY,
		QM_MATCH_RIGHT,
		QM_MATCH_RIGHT_NO_PARAM
	} eState = INIT;

	for(; pWhere[0] != 0; ++pWhere){
		switch(eState){
			case INIT:
				if(pWhere[0] == '<'){
					eState = LT_MATCH;
				} else {
					return nullptr;
				}
				break;
			case LT_MATCH:
				if(pWhere[0] == '?'){
					eState = QM_MATCH_LEFT;
				} else {
					return nullptr;
				}
				break;
			case QM_MATCH_LEFT:
				if(CanBeginIDs(pWhere[0])){
					*ppInstTypeBegin = pWhere;
					eState = INST_NAME_BODY;
				} else {
					return nullptr;
				}
				break;
			case INST_NAME_BODY:
				if(pWhere[0] == '?'){
					*ppInstTypeEnd = pWhere;
					*ppInstParamBegin = pWhere;
					*ppInstParamEnd = pWhere;
					eState = QM_MATCH_RIGHT_NO_PARAM;
				} else if(IsBlank(pWhere[0])){
					*ppInstTypeEnd = pWhere;
					eState = INST_NAME_ACCEPTED;
				} else if(!IsLegalInIDs(pWhere[0])){
					return nullptr;
				}
				break;
			case INST_NAME_ACCEPTED:
				*ppInstParamBegin = pWhere;
				if(pWhere[0] == '?'){
					eState = QM_MATCH_RIGHT;
				} else if(!IsBlank(pWhere[0])){
					eState = INST_PARAM_BODY;
				}
				break;
			case INST_PARAM_BODY:
				if(pWhere[0] == '?'){
					eState = QM_MATCH_RIGHT;
				}
				break;
			case QM_MATCH_RIGHT:
				if(pWhere[0] == '>'){
					*ppInstParamEnd = pWhere - 1;
					return pWhere + 1;
				} else {
					eState = INST_PARAM_BODY;
				}
				break;
			case QM_MATCH_RIGHT_NO_PARAM:
				if(pWhere[0] == '>'){
					*ppInstParamEnd = pWhere - 1;
					return pWhere + 1;
				} else {
					return nullptr;
				}
				break;
		}
	}
	return nullptr;
}
const char *XMLDocumentClass::XMLElementClass::xXMLParserClass::xTryDocType(const char *pWhere, const char **ppRootElementKeyBegin, const char **ppRootElementKeyEnd, const char **ppSourceTypeBegin, const char **ppSourceTypeEnd, const char **ppIdentifierBegin, const char **ppIdentifierEnd, const char **ppSourceURIBegin, const char **ppSourceURIEnd, const char **ppSubsetBegin, const char **ppSubsetEnd){
	// XML DOCTYPE 正则表达式		 <!DOCTYPE\s+\w[\w\.\-_:α]*(\s+((SYSTEM)|(PUBLIC\s+('.*?')|(".*?")))\s+('.*?')|(".*?"))?(\s*\[.*?\])?\s*>
	//								|||||||||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// DFA 状态集合					|||||||||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	//								|||||||||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// INIT						----/||||||||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// LT_MATCH					-----/|||||||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EM_MATCH					------/||||||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// D_MATCH					-------/|||||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// O_MATCH					--------/||||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// C_MATCH					---------/|||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// T_MATCH					----------/||| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// Y_MATCH					-----------/|| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// P_MATCH					------------/| |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// E_MATCH					-------------/ |  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// DOCTYPE_ACCEPTED			---------------/  |           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// ROOT_KEY_BODY			------------------/           |   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// (ROOT_KEY_BODY)			------------------------------/   |   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// ROOT_KEY_ACCEPTED		----------------------------------/   ||||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX1_S_MATCH_1ST			--------------------------------------/|||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX1_Y_MATCH				---------------------------------------/||||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX1_S_MATCH_2ND			----------------------------------------/|||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX1_T_MATCH				-----------------------------------------/||   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX1_E_MATCH				------------------------------------------/|   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX1_M_MATCH				-------------------------------------------/   |||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX2_P_MATCH				-----------------------------------------------/||||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX2_U_MATCH				------------------------------------------------/|||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX2_B_MATCH				-------------------------------------------------/||| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX2_L_MATCH				--------------------------------------------------/|| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX2_I_MATCH				---------------------------------------------------/| |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX2_C_MATCH				----------------------------------------------------/ |  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX2_PUBLIC_ACCEPTED		------------------------------------------------------/  ||  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX2_ID_APOS				---------------------------------------------------------/|  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// (EX2_ID_APOS)			----------------------------------------------------------/  |   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX_TYPE_ACCEPTED			-------------------------------------------------------------/   ||  |    |  ||  |   ||  |     |  ||   |   | |
	// EX2_ID_QUOT				-----------------------------------------------------------------/|  |    |  ||  |   ||  |     |  ||   |   | |
	// (EX2_ID_QUOT)			------------------------------------------------------------------/  |    |  ||  |   ||  |     |  ||   |   | |
	// (EX_TYPE_ACCEPTED)		---------------------------------------------------------------------/    |  ||  |   ||  |     |  ||   |   | |
	// (EX_TYPE_ACCEPTED)		--------------------------------------------------------------------------/  ||  |   ||  |     |  ||   |   | |
	// SRC_URI_APOS				-----------------------------------------------------------------------------/|  |   ||  |     |  ||   |   | |
	// (SRC_URI_APOS)			------------------------------------------------------------------------------/  |   ||  |     |  ||   |   | |
	// SRC_ACCEPTED				---------------------------------------------------------------------------------/   ||  |     |  ||   |   | |
	// SRC_URI_QUOT				-------------------------------------------------------------------------------------/|  |     |  ||   |   | |
	// (SRC_URI_QUOT)			--------------------------------------------------------------------------------------/  |     |  ||   |   | |
	// (SRC_ACCEPTED)			-----------------------------------------------------------------------------------------/     |  ||   |   | |
	// (SRC_ACCEPTED)			-----------------------------------------------------------------------------------------------/  ||   |   | |
	// SUBSET_SB_MATCH_LEFT		--------------------------------------------------------------------------------------------------/|   |   | |
	// (SUBSET_SB_MATCH_LEFT)	---------------------------------------------------------------------------------------------------/   |   | |
	// SUBSET_SB_MATCH_RIGHT	-------------------------------------------------------------------------------------------------------/   | |
	// (SUBSET_SB_MATCH_RIGHT)	-----------------------------------------------------------------------------------------------------------/ |
	// (GT_MATCH)				-------------------------------------------------------------------------------------------------------------/

	enum {
		INIT,
		LT_MATCH,
		EM_MATCH,
		D_MATCH,
		O_MATCH,
		C_MATCH,
		T_MATCH,
		Y_MATCH,
		P_MATCH,
		E_MATCH,
		DOCTYPE_ACCEPTED,
		ROOT_KEY_BODY,
		ROOT_KEY_ACCEPTED,
		EX1_S_MATCH_1ST,
		EX1_Y_MATCH,
		EX1_S_MATCH_2ND,
		EX1_T_MATCH,
		EX1_E_MATCH,
		EX1_M_MATCH,
		EX2_P_MATCH,
		EX2_U_MATCH,
		EX2_B_MATCH,
		EX2_L_MATCH,
		EX2_I_MATCH,
		EX2_C_MATCH,
		EX2_PUBLIC_ACCEPTED,
		EX2_ID_APOS,
		EX_TYPE_ACCEPTED,
		EX2_ID_QUOT,
		SRC_URI_APOS,
		SRC_URI_QUOT,
		SRC_ACCEPTED,
		SUBSET_SB_MATCH_LEFT,
		SUBSET_SB_MATCH_RIGHT
	} eState = INIT;

	for(; pWhere[0] != 0; ++pWhere){
		switch(eState){
			case INIT:
				if(pWhere[0] == '<'){
					eState = LT_MATCH;
				} else {
					return nullptr;
				}
				break;
			case LT_MATCH:
				if(pWhere[0] == '!'){
					eState = EM_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EM_MATCH:
				if(pWhere[0] == 'D'){
					eState = D_MATCH;
				} else {
					return nullptr;
				}
				break;
			case D_MATCH:
				if(pWhere[0] == 'O'){
					eState = O_MATCH;
				} else {
					return nullptr;
				}
				break;
			case O_MATCH:
				if(pWhere[0] == 'C'){
					eState = C_MATCH;
				} else {
					return nullptr;
				}
				break;
			case C_MATCH:
				if(pWhere[0] == 'T'){
					eState = T_MATCH;
				} else {
					return nullptr;
				}
				break;
			case T_MATCH:
				if(pWhere[0] == 'Y'){
					eState = Y_MATCH;
				} else {
					return nullptr;
				}
				break;
			case Y_MATCH:
				if(pWhere[0] == 'P'){
					eState = P_MATCH;
				} else {
					return nullptr;
				}
				break;
			case P_MATCH:
				if(pWhere[0] == 'E'){
					eState = E_MATCH;
				} else {
					return nullptr;
				}
				break;
			case E_MATCH:
				if(IsBlank(pWhere[0])){
					eState = DOCTYPE_ACCEPTED;
				} else {
					return nullptr;
				}
				break;
			case DOCTYPE_ACCEPTED:
				if(CanBeginIDs(pWhere[0])){
					*ppRootElementKeyBegin = pWhere;
					eState = ROOT_KEY_BODY;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
			case ROOT_KEY_BODY:
				if(IsBlank(pWhere[0])){
					*ppRootElementKeyEnd = pWhere;
					eState = ROOT_KEY_ACCEPTED;
				} else if(pWhere[0] == '>'){
					*ppRootElementKeyEnd = pWhere;
					*ppSourceTypeBegin = pWhere;
					*ppSourceTypeEnd = pWhere;
					*ppIdentifierBegin = pWhere;
					*ppIdentifierEnd = pWhere;
					*ppSourceURIBegin = pWhere;
					*ppSourceURIEnd = pWhere;
					*ppSubsetBegin = pWhere;
					*ppSubsetEnd = pWhere;
					return pWhere + 1;
				} else if(!IsLegalInIDs(pWhere[0])){
					return nullptr;
				}
				break;
			case ROOT_KEY_ACCEPTED:
				if(pWhere[0] == 'S'){
					*ppSourceTypeBegin = pWhere;
					eState = EX1_S_MATCH_1ST;
				} else if(pWhere[0] == 'P'){
					*ppSourceTypeBegin = pWhere;
					eState = EX2_P_MATCH;
				} else if(pWhere[0] == '['){
					*ppSourceTypeBegin = pWhere;
					*ppSourceTypeEnd = pWhere;
					*ppIdentifierBegin = pWhere;
					*ppIdentifierEnd = pWhere;
					*ppSourceURIBegin = pWhere;
					*ppSourceURIEnd = pWhere;
					*ppSubsetBegin = pWhere + 1;
					eState = SUBSET_SB_MATCH_LEFT;
				} else if(pWhere[0] == '>'){
					*ppSourceTypeBegin = pWhere;
					*ppSourceTypeEnd = pWhere;
					*ppIdentifierBegin = pWhere;
					*ppIdentifierEnd = pWhere;
					*ppSourceURIBegin = pWhere;
					*ppSourceURIEnd = pWhere;
					*ppSubsetBegin = pWhere;
					*ppSubsetEnd = pWhere;
					return pWhere + 1;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
			case EX1_S_MATCH_1ST:
				if(pWhere[0] == 'Y'){
					eState = EX1_Y_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EX1_Y_MATCH:
				if(pWhere[0] == 'S'){
					eState = EX1_S_MATCH_2ND;
				} else {
					return nullptr;
				}
				break;
			case EX1_S_MATCH_2ND:
				if(pWhere[0] == 'T'){
					eState = EX1_T_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EX1_T_MATCH:
				if(pWhere[0] == 'E'){
					eState = EX1_E_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EX1_E_MATCH:
				if(pWhere[0] == 'M'){
					eState = EX1_M_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EX1_M_MATCH:
				if(IsBlank(pWhere[0])){
					*ppSourceTypeEnd = pWhere;
					*ppIdentifierBegin = pWhere;
					*ppIdentifierEnd = pWhere;
					eState = EX_TYPE_ACCEPTED;
				} else {
					return nullptr;
				}
				break;
			case EX2_P_MATCH:
				if(pWhere[0] == 'U'){
					eState = EX2_U_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EX2_U_MATCH:
				if(pWhere[0] == 'B'){
					eState = EX2_B_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EX2_B_MATCH:
				if(pWhere[0] == 'L'){
					eState = EX2_L_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EX2_L_MATCH:
				if(pWhere[0] == 'I'){
					eState = EX2_I_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EX2_I_MATCH:
				if(pWhere[0] == 'C'){
					eState = EX2_C_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EX2_C_MATCH:
				if(IsBlank(pWhere[0])){
					*ppSourceTypeEnd = pWhere;
					eState = EX2_PUBLIC_ACCEPTED;
				} else {
					return nullptr;
				}
				break;
			case EX2_PUBLIC_ACCEPTED:
				if(pWhere[0] == '\''){
					*ppIdentifierBegin = pWhere + 1;
					eState = EX2_ID_APOS;
				} else if(pWhere[0] == '"'){
					*ppIdentifierBegin = pWhere + 1;
					eState = EX2_ID_QUOT;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
			case EX2_ID_APOS:
				if(pWhere[0] == '\''){
					*ppIdentifierEnd = pWhere;
					eState = EX_TYPE_ACCEPTED;
				}
				break;
			case EX_TYPE_ACCEPTED:
				if(pWhere[0] == '\''){
					*ppSourceURIBegin = pWhere + 1;
					eState = SRC_URI_APOS;
				} else if(pWhere[0] == '"'){
					*ppSourceURIBegin = pWhere + 1;
					eState = SRC_URI_QUOT;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
			case EX2_ID_QUOT:
				if(pWhere[0] == '\"'){
					*ppIdentifierEnd = pWhere;
					eState = EX_TYPE_ACCEPTED;
				}
				break;
			case SRC_URI_APOS:
				if(pWhere[0] == '\''){
					*ppSourceURIEnd = pWhere;
					eState = SRC_ACCEPTED;
				}
				break;
			case SRC_URI_QUOT:
				if(pWhere[0] == '"'){
					*ppSourceURIEnd = pWhere;
					eState = SRC_ACCEPTED;
				}
				break;
			case SRC_ACCEPTED:
				if(pWhere[0] == '['){
					*ppSubsetBegin = pWhere + 1;
					eState = SUBSET_SB_MATCH_LEFT;
				} else if(pWhere[0] == '>'){
					*ppSubsetBegin = pWhere;
					*ppSubsetEnd = pWhere;
					return pWhere + 1;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
			case SUBSET_SB_MATCH_LEFT:
				if(pWhere[0] == ']'){
					*ppSubsetEnd = pWhere;
					eState = SUBSET_SB_MATCH_RIGHT;
				}
				break;
			case SUBSET_SB_MATCH_RIGHT:
				if(pWhere[0] == '>'){
					return pWhere + 1;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
		}
	}
	return nullptr;
}
const char *XMLDocumentClass::XMLElementClass::xXMLParserClass::xTryCData(const char *pWhere, const char **ppCDataBegin, const char **ppCDataEnd){
	// XML CDATA 标签正则表达式		 <!\[CDATA\[.*?\]\]>
	//								||| |||||| ||   | ||
	// DFA 状态集合					||| |||||| ||   | ||
	//								||| |||||| ||   | ||
	// INIT						----/|| |||||| ||   | ||
	// LT_MATCH					-----/| |||||| ||   | ||
	// EM_MATCH					------/ |||||| ||   | ||
	// SB_MATCH_LEFT_1ST		--------/||||| ||   | ||
	// C_MATCH					---------/|||| ||   | ||
	// D_MATCH					----------/||| ||   | ||
	// A_MATCH_1ST				-----------/|| ||   | ||
	// T_MATCH					------------/| ||   | ||
	// A_MATCH_2ND				-------------/ ||   | ||
	// SB_MATCH_LEFT_2ND		---------------/|   | ||
	// CDATA_BODY				----------------/   | ||
	// SB_MATCH_RIGHT_2ND		--------------------/ ||
	// SB_MATCH_RIGHT_1ST		----------------------/|
	// (GT_MATCH)				-----------------------/

	enum {
		INIT,
		LT_MATCH,
		EM_MATCH,
		SB_MATCH_LEFT_1ST,
		C_MATCH,
		D_MATCH,
		A_MATCH_1ST,
		T_MATCH,
		A_MATCH_2ND,
		SB_MATCH_LEFT_2ND,
		CDATA_BODY,
		SB_MATCH_RIGHT_2ND,
		SB_MATCH_RIGHT_1ST,
	} eState = INIT;

	for(; pWhere[0] != 0; ++pWhere){
		switch(eState){
			case INIT:
				if(pWhere[0] == '<'){
					eState = LT_MATCH;
				} else {
					return nullptr;
				}
				break;
			case LT_MATCH:
				if(pWhere[0] == '!'){
					eState = EM_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EM_MATCH:
				if(pWhere[0] == '['){
					eState = SB_MATCH_LEFT_1ST;
				} else {
					return nullptr;
				}
				break;
			case SB_MATCH_LEFT_1ST:
				if(pWhere[0] == 'C'){
					eState = C_MATCH;
				} else {
					return nullptr;
				}
				break;
			case C_MATCH:
				if(pWhere[0] == 'D'){
					eState = D_MATCH;
				} else {
					return nullptr;
				}
				break;
			case D_MATCH:
				if(pWhere[0] == 'A'){
					eState = A_MATCH_1ST;
				} else {
					return nullptr;
				}
				break;
			case A_MATCH_1ST:
				if(pWhere[0] == 'T'){
					eState = T_MATCH;
				} else {
					return nullptr;
				}
				break;
			case T_MATCH:
				if(pWhere[0] == 'A'){
					eState = A_MATCH_2ND;
				} else {
					return nullptr;
				}
				break;
			case A_MATCH_2ND:
				if(pWhere[0] == '['){
					eState = SB_MATCH_LEFT_2ND;
				} else {
					return nullptr;
				}
				break;
			case SB_MATCH_LEFT_2ND:
				*ppCDataBegin = pWhere;
				if(pWhere[0] == ']'){
					eState = SB_MATCH_RIGHT_2ND;
				} else {
					eState = CDATA_BODY;
				}
				break;
			case CDATA_BODY:
				if(pWhere[0] == ']'){
					eState = SB_MATCH_RIGHT_2ND;
				}
				break;
			case SB_MATCH_RIGHT_2ND:
				if(pWhere[0] == ']'){
					eState = SB_MATCH_RIGHT_1ST;
				} else {
					eState = CDATA_BODY;
				}
				break;
			case SB_MATCH_RIGHT_1ST:
				if(pWhere[0] == '>'){
					*ppCDataEnd = pWhere - 2;
					return pWhere + 1;
				} else {
					eState = CDATA_BODY;
				}
				break;
		}
	}
	return nullptr;
}
const char *XMLDocumentClass::XMLElementClass::xXMLParserClass::xTryComment(const char *pWhere, const char **ppCommentBegin, const char **ppCommentEnd){
	// XML 注释正则表达式			 <!--.*?-->
	//								||||||  |||
	// DFA 状态集合					||||||  |||
	//								||||||  |||
	// INIT						----/|||||  |||
	// LT_MATCH					-----/||||  |||
	// EM_MATCH					------/|||  |||
	// MNS_MATCH_1ST			-------/||  |||
	// MNS_MATCH_2ND			--------/|  |||
	// COMMENT_BODY				---------/  |||
	// MNS_MATCH_3RD			------------/||
	// MNS_MATCH_4TH			-------------/|
	// (GT_MATCH)				--------------/

	enum {
		INIT,
		LT_MATCH,
		EM_MATCH,
		MNS_MATCH_1ST,
		MNS_MATCH_2ND,
		COMMENT_BODY,
		MNS_MATCH_3RD,
		MNS_MATCH_4TH
	} eState = INIT;

	for(; pWhere[0] != 0; ++pWhere){
		switch(eState){
			case INIT:
				if(pWhere[0] == '<'){
					eState = LT_MATCH;
				} else {
					return nullptr;
				}
				break;
			case LT_MATCH:
				if(pWhere[0] == '!'){
					eState = EM_MATCH;
				} else {
					return nullptr;
				}
				break;
			case EM_MATCH:
				if(pWhere[0] == '-'){
					eState = MNS_MATCH_1ST;
				} else {
					return nullptr;
				}
				break;
			case MNS_MATCH_1ST:
				if(pWhere[0] == '-'){
					eState = MNS_MATCH_2ND;
				} else {
					return nullptr;
				}
				break;
			case MNS_MATCH_2ND:
				*ppCommentBegin = pWhere;
				if(pWhere[0] == '-'){
					eState = MNS_MATCH_3RD;
				} else {
					eState = COMMENT_BODY;
				}
				break;
			case COMMENT_BODY:
				if(pWhere[0] == '-'){
					eState = MNS_MATCH_3RD;
				}
				break;
			case MNS_MATCH_3RD:
				if(pWhere[0] == '-'){
					eState = MNS_MATCH_4TH;
				} else {
					eState = COMMENT_BODY;
				}
				break;
			case MNS_MATCH_4TH:
				if(pWhere[0] == '>'){
					*ppCommentEnd = pWhere - 2;
					return pWhere + 1;
				} else {
					eState = COMMENT_BODY;
				}
				break;
		}
	}
	return nullptr;
}
const char *XMLDocumentClass::XMLElementClass::xXMLParserClass::xTryElementOpen(const char *pWhere, const char **ppKeyBegin, const char **ppKeyEnd, std::vector<xATTRIB_POS> *pvecAttributes, bool *pbIsComplete){
	// XML 元素开启正则表达式		 <\w[\w\.\-_:α]*(\s+\w[\w\.\-_:α]*\s*=\s*(('.*?')|(".*?")))*\s*/?>
	//								|| |           |   |  |           |  | | |   ||  |   ||  |     | | |
	// DFA 状态集合					|| |           |   |  |           |  | | |   ||  |   ||  |     | | |
	//								|| |           |   |  |           |  | | |   ||  |   ||  |     | | |
	// INIT						----/| |           |   |  |           |  | | |   ||  |   ||  |     | | |
	// LT_MATCH					-----/ |           |   |  |           |  | | |   ||  |   ||  |     | | |
	// KEY_BODY					-------/           |   |  |           |  | | |   ||  |   ||  |     | | |
	// (KEY_BODY)				-------------------/   |  |           |  | | |   ||  |   ||  |     | | |
	// KEY_ACCEPTED				-----------------------/  |           |  | | |   ||  |   ||  |     | | |
	// ATTRIB_NAME_BODY			--------------------------/           |  | | |   ||  |   ||  |     | | |
	// (ATTRIB_NAME_BODY)		--------------------------------------/  | | |   ||  |   ||  |     | | |
	// ATTRIB_NAME_ACCEPTED		-----------------------------------------/ | |   ||  |   ||  |     | | |
	// ATTRIB_EQU_MATCH			-------------------------------------------/ |   ||  |   ||  |     | | |
	// (ATTRIB_EQU_MATCH)		---------------------------------------------/   ||  |   ||  |     | | |
	// ATTRIB_APOS_VAL			-------------------------------------------------/|  |   ||  |     | | |
	// (ATTRIB_APOS_VAL)		--------------------------------------------------/  |   ||  |     | | |
	// (KEY_ACCEPTED)			-----------------------------------------------------/   ||  |     | | |
	// ATTRIB_QUOT_VAL			---------------------------------------------------------/|  |     | | |
	// (ATTRIB_QUOT_VAL)		----------------------------------------------------------/  |     | | |
	// (KEY_ACCEPTED)			-------------------------------------------------------------/     | | |
	// (KEY_ACCEPTED)			-------------------------------------------------------------------/ | |
	// SL_MATCH					---------------------------------------------------------------------/ |
	// (GT_MATCH)				-----------------------------------------------------------------------/

	pvecAttributes->clear();

	enum {
		INIT,
		LT_MATCH,
		KEY_BODY,
		KEY_ACCEPTED,
		ATTRIB_NAME_BODY,
		ATTRIB_NAME_ACCEPTED,
		ATTRIB_EQU_MATCH,
		ATTRIB_APOS_VAL,
		ATTRIB_QUOT_VAL,
		SL_MATCH
	} eState = INIT;

	for(; pWhere[0] != 0; ++pWhere){
		switch(eState){
			case INIT:
				if(pWhere[0] == '<'){
					eState = LT_MATCH;
				} else {
					return nullptr;
				}
				break;
			case LT_MATCH:
				if(CanBeginIDs(pWhere[0])){
					*ppKeyBegin = pWhere;
					eState = KEY_BODY;
				} else {
					return nullptr;
				}
				break;
			case KEY_BODY:
				if(pWhere[0] == '/'){
					*ppKeyEnd = pWhere;
					*pbIsComplete = true;
					eState = SL_MATCH;
				} else if(pWhere[0] == '>'){
					*ppKeyEnd = pWhere;
					*pbIsComplete = false;
					return pWhere + 1;
				} else if(IsBlank(pWhere[0])){
					*ppKeyEnd = pWhere;
					eState = KEY_ACCEPTED;
				} else if(!IsLegalInIDs(pWhere[0])){
					return nullptr;
				}
				break;
			case KEY_ACCEPTED:
				if(pWhere[0] == '/'){
					*pbIsComplete = true;
					eState = SL_MATCH;
				} else if(pWhere[0] == '>'){
					*pbIsComplete = false;
					return pWhere + 1;
				} else if(CanBeginIDs(pWhere[0])){
					pvecAttributes->emplace_back(xATTRIB_POS());
					pvecAttributes->back().pNameBegin = pWhere;
					eState = ATTRIB_NAME_BODY;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
			case ATTRIB_NAME_BODY:
				if(pWhere[0] == '='){
					pvecAttributes->back().pNameEnd = pWhere;
					eState = ATTRIB_EQU_MATCH;
				} else if(IsBlank(pWhere[0])){
					pvecAttributes->back().pNameEnd = pWhere;
					eState = ATTRIB_NAME_ACCEPTED;
				} else if(!IsLegalInIDs(pWhere[0])){
					return nullptr;
				}
				break;
			case ATTRIB_NAME_ACCEPTED:
				if(pWhere[0] == '='){
					eState = ATTRIB_EQU_MATCH;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
			case ATTRIB_EQU_MATCH:
				if(pWhere[0] == '\''){
					pvecAttributes->back().pValBegin = pWhere + 1;
					eState = ATTRIB_APOS_VAL;
				} else if(pWhere[0] == '"'){
					pvecAttributes->back().pValBegin = pWhere + 1;
					eState = ATTRIB_QUOT_VAL;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
			case ATTRIB_APOS_VAL:
				if(pWhere[0] == '\''){
					pvecAttributes->back().pValEnd = pWhere;
					eState = KEY_ACCEPTED;
				}
				break;
			case ATTRIB_QUOT_VAL:
				if(pWhere[0] == '\"'){
					pvecAttributes->back().pValEnd = pWhere;
					eState = KEY_ACCEPTED;
				}
				break;
			case SL_MATCH:
				if(pWhere[0] == '>'){
					return pWhere + 1;
				} else {
					return nullptr;
				}
				break;
		}
	}
	return nullptr;
}
const char *XMLDocumentClass::XMLElementClass::xXMLParserClass::xTryElementClose(const char *pWhere, const char **ppKeyBegin, const char **ppKeyEnd){
	// XML 元素关闭正则表达式		 </\w[\w\.\-_:α]*\s*>
	//								||| |           |  | |
	// DFA 状态集合					||| |           |  | |
	//								||| |           |  | |
	// INIT						----/|| |           |  | |
	// LT_MATCH					-----/| |           |  | |
	// SL_MATCH					------/ |           |  | |
	// KEY_BODY					--------/           |  | |
	// (KEY_BODY)				--------------------/  | |
	// KEY_ACCEPTED				-----------------------/ |
	// (GT_MATCH)				-------------------------/

	enum {
		INIT,
		LT_MATCH,
		SL_MATCH,
		KEY_BODY,
		KEY_ACCEPTED
	} eState = INIT;

	for(; pWhere[0] != 0; ++pWhere){
		switch(eState){
			case INIT:
				if(pWhere[0] == '<'){
					eState = LT_MATCH;
				} else {
					return nullptr;
				}
				break;
			case LT_MATCH:
				if(pWhere[0] == '/'){
					eState = SL_MATCH;
				} else {
					return nullptr;
				}
				break;
			case SL_MATCH:
				if(CanBeginIDs(pWhere[0])){
					*ppKeyBegin = pWhere;
					eState = KEY_BODY;
				} else {
					return nullptr;
				}
				break;
			case KEY_BODY:
				if(pWhere[0] == '>'){
					*ppKeyEnd = pWhere;
					return pWhere + 1;
				} else if(IsBlank(pWhere[0])){
					*ppKeyEnd = pWhere;
					eState = KEY_ACCEPTED;
				} else if(!IsLegalInIDs(pWhere[0])){
					return nullptr;
				}
				break;
			case KEY_ACCEPTED:
				if(pWhere[0] == '>'){
					return pWhere + 1;
				} else if(!IsBlank(pWhere[0])){
					return nullptr;
				}
				break;
		}
	}
	return nullptr;
}

inline __attribute__((always_inline)) bool XMLDocumentClass::XMLElementClass::xXMLParserClass::CanBeginIDs(char ch){
	return (('a' <= ch) && (ch <= 'z')) ||
		(('A' <= ch) && (ch <= 'Z')) ||
		(ch == '_') ||
		(((unsigned char)ch) >= 0x80);
}
inline __attribute__((always_inline)) bool XMLDocumentClass::XMLElementClass::xXMLParserClass::IsLegalInIDs(char ch){
	switch(ch){
		case '.':
		case '_':
		case '-':
		case ':':
			return true;
		default:
			return CanBeginIDs(ch);
	}
}
inline __attribute__((always_inline)) bool XMLDocumentClass::XMLElementClass::xXMLParserClass::IsBlank(char ch){
	switch(ch){
		case '\r':
		case '\n':
		case ' ':
		case '\t':
			return true;
		default:
			return false;
	}
}

// 构造函数和析构函数。
XMLDocumentClass::XMLElementClass::xXMLParserClass::xXMLParserClass(){
	xm_pRoot = nullptr;
	xm_pCurrent = nullptr;
}
XMLDocumentClass::XMLElementClass::xXMLParserClass::~xXMLParserClass(){
	delete xm_pRoot;
}

// 其他非静态成员函数。
void XMLDocumentClass::XMLElementClass::xXMLParserClass::xInit(){
	delete xm_pRoot;
	xm_pRoot = nullptr;
	xm_pCurrent = nullptr;
	xm_lstKeyStack.clear();

	xm_eErrorCode = ERR_NONE;
	xm_strErrorInfo.clear();
}

void XMLDocumentClass::XMLElementClass::xXMLParserClass::xOnInstruction(const char *pInstTypeBegin, const char *pInstTypeEnd, const char *pInstParamBegin, const char *pInstParamEnd){
	UNREF_PARAM(pInstTypeBegin);
	UNREF_PARAM(pInstTypeEnd);
	UNREF_PARAM(pInstParamBegin);
	UNREF_PARAM(pInstParamEnd);
}
void XMLDocumentClass::XMLElementClass::xXMLParserClass::xOnDocType(const char *pRootElementKeyBegin, const char *pRootElementKeyEnd, const char *pSourceTypeBegin, const char *pSourceTypeEnd, const char *pIdentifierBegin, const char *pIdentifierEnd, const char *pSourceURIBegin, const char *pSourceURIEnd, const char *pSubsetBegin, const char *pSubsetEnd){
	// ID 对于 SYSTEM 来源的 DTD 无意义。
	UNREF_PARAM(pRootElementKeyBegin);
	UNREF_PARAM(pRootElementKeyEnd);
	UNREF_PARAM(pSourceTypeBegin);
	UNREF_PARAM(pSourceTypeEnd);
	UNREF_PARAM(pIdentifierBegin);
	UNREF_PARAM(pIdentifierEnd);
	UNREF_PARAM(pSourceURIBegin);
	UNREF_PARAM(pSourceURIEnd);
	UNREF_PARAM(pSubsetBegin);
	UNREF_PARAM(pSubsetEnd);
}
void XMLDocumentClass::XMLElementClass::xXMLParserClass::xOnCData(const char *pCDataBegin, const char *pCDataEnd){
	if(xm_pCurrent == nullptr){
		for(const char *pTest = pCDataBegin; pTest != pCDataEnd; ++pTest){
			if(!IsBlank(*pTest)){
				throw xPARSE_ERROR(ERR_DISSOCIATIVE_CDATA, pTest);
			}
		}
	} else {
		std::string strTranslated;
		const char *const pErrorPos = xRemoveEntities(strTranslated, pCDataBegin, pCDataEnd);
		if(pErrorPos != nullptr){
			throw xPARSE_ERROR(ERR_INVALID_ENTITY, pErrorPos);
		}
		xm_pCurrent->GetCData().append(strTranslated);
	}
}
void XMLDocumentClass::XMLElementClass::xXMLParserClass::xOnComment(const char *pCommentBegin, const char *pCommentEnd){
	UNREF_PARAM(pCommentBegin);
	UNREF_PARAM(pCommentEnd);
}
void XMLDocumentClass::XMLElementClass::xXMLParserClass::xOnElementOpen(const char *pKeyBegin, const char *pKeyEnd, const std::vector<xATTRIB_POS> &vecAttributes){
	if(xm_pCurrent == nullptr){
		if(xm_pRoot != nullptr){
			throw xPARSE_ERROR(ERR_MULTIPLE_ROOT_ELEMENT, pKeyBegin, xm_pRoot->xm_strKey);
		}
		xm_pRoot = new XMLElementClass(nullptr, std::string(pKeyBegin, pKeyEnd));
		xm_pCurrent = xm_pRoot;
	} else {
		xm_pCurrent = xm_pCurrent->AppendChild(std::string(pKeyBegin, pKeyEnd));
	}
	xm_lstKeyStack.emplace_front(std::string(pKeyBegin, pKeyEnd));

	for(auto iter = vecAttributes.cbegin(); iter != vecAttributes.cend(); ++iter){
		std::string strTranslatedAttribVal;
		const char *const pErrorPos = xRemoveEntities(strTranslatedAttribVal, iter->pValBegin, iter->pValEnd);
		if(pErrorPos != nullptr){
			throw xPARSE_ERROR(ERR_INVALID_ENTITY, pErrorPos);
		}
		xm_pCurrent->xm_mapAttributes.emplace(std::make_pair(std::string(iter->pNameBegin, iter->pNameEnd), std::move(strTranslatedAttribVal)));
	}
}
void XMLDocumentClass::XMLElementClass::xXMLParserClass::xOnElementClose(const char *pKeyBegin, const char *pKeyEnd){
	if(xm_lstKeyStack.empty()){
		throw xPARSE_ERROR(ERR_DISSOCIATIVE_ELEMENT_CLOSEtag, pKeyBegin);
	} else if((xm_lstKeyStack.front().size() != (std::size_t)(pKeyEnd - pKeyBegin)) || (std::memcmp(xm_lstKeyStack.front().c_str(), pKeyBegin, (std::size_t)(pKeyEnd - pKeyBegin)) != 0)){
		throw xPARSE_ERROR(ERR_ELEMENT_CLOSEtag_MISMATCH, pKeyBegin, xm_lstKeyStack.front());
	}
	xm_lstKeyStack.pop_front();
	xm_pCurrent = xm_pCurrent->GetParent();
}

const char *XMLDocumentClass::XMLElementClass::xXMLParserClass::Parse(const char *pszXMLString){
	xInit();

	try {
		const char *pRead = pszXMLString;
		while(*pRead != 0){
			if(*pRead == '<'){
				const char *pNextPos, *pBegin, *pEnd, *pParamBegin, *pParamEnd;
				const char *pIdentifierBegin, *pIdentifierEnd, *pSourceURIBegin, *pSourceURIEnd, *pSubsetBegin, *pSubsetEnd;
				std::vector<xATTRIB_POS> vecAttribPos;
				bool bIsComplete;

				if((pNextPos = xTryInstruction(pRead, &pBegin, &pEnd, &pParamBegin, &pParamEnd)) != nullptr){
					xOnInstruction(pBegin, pEnd, pParamBegin, pParamEnd);
				} else if((pNextPos = xTryDocType(pRead, &pBegin, &pEnd, &pParamBegin, &pParamEnd, &pIdentifierBegin, &pIdentifierEnd, &pSourceURIBegin, &pSourceURIEnd, &pSubsetBegin, &pSubsetEnd)) != nullptr){
					xOnDocType(pBegin, pEnd, pParamBegin, pParamEnd, pIdentifierBegin, pIdentifierEnd, pSourceURIBegin, pSourceURIEnd, pSubsetBegin, pSubsetEnd);
				} else if((pNextPos = xTryCData(pRead, &pBegin, &pEnd)) != nullptr){
					xOnCData(pBegin, pEnd);
				} else if((pNextPos = xTryComment(pRead, &pBegin, &pEnd)) != nullptr){
					xOnComment(pBegin, pEnd);
				} else if((pNextPos = xTryElementOpen(pRead, &pBegin, &pEnd, &vecAttribPos, &bIsComplete)) != nullptr){
					xOnElementOpen(pBegin, pEnd, vecAttribPos);
					if(bIsComplete){
						xOnElementClose(pBegin, pEnd);
					}
				} else if((pNextPos = xTryElementClose(pRead, &pBegin, &pEnd)) != nullptr){
					xOnElementClose(pBegin, pEnd);
				} else {
					throw xPARSE_ERROR(ERR_UNRECOGNIZEDtag, pRead);
				}
				pRead = pNextPos;
			} else {
				const char *pCDataEnd = pRead + 1;
				while((*pCDataEnd != 0) && (*pCDataEnd != '<')){
					++pCDataEnd;
				}
				xOnCData(pRead, pCDataEnd);
				pRead = pCDataEnd;
			}
		}

		if(!xm_lstKeyStack.empty()){
			throw xPARSE_ERROR(ERR_MISSING_ELEMENT_CLOSEtag, pRead, std::move(xm_lstKeyStack.front()));
		}
		return nullptr;
	} catch(xPARSE_ERROR ParseException){
		xm_eErrorCode = ParseException.eErrorCode;
		xm_strErrorInfo = std::move(ParseException.strErrorInfo);
		return ParseException.pWhere;
	}
}

XMLDocumentClass::XMLElementClass::xXMLParserClass::ERROR_CODE XMLDocumentClass::XMLElementClass::xXMLParserClass::GetErrorCode() const {
	return xm_eErrorCode;
}
const std::string &XMLDocumentClass::XMLElementClass::xXMLParserClass::GetErrorInfo() const {
	return xm_strErrorInfo;
}
XMLDocumentClass::XMLElementClass *XMLDocumentClass::XMLElementClass::xXMLParserClass::GetMovableRootElement(){
	return xm_pRoot;
}
