// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Notation.hpp"
#include "../Containers/MultiIndexMap.hpp"
#include "../Containers/VVector.hpp"
using namespace MCF;

namespace {

WideString Unescape(const WideStringObserver &wsoSrc){
	WideString wcsRet;
	wcsRet.Reserve(wsoSrc.GetSize());

	enum {
		NORMAL,
		ESCAPED,
		UCS_CODE
	} eState = NORMAL;

	std::uint32_t u32CodePoint = 0;
	std::size_t uHexExpecting = 0;

	const auto PushUtf = [&]{
		if(u32CodePoint > 0x10FFFF){
			u32CodePoint = 0xFFFD;
		}
		if(u32CodePoint <= 0xFFFF){
			wcsRet.Append(u32CodePoint);
		} else {
			u32CodePoint -= 0x10000;
			wcsRet.Append((u32CodePoint >> 10)   | 0xD800);
			wcsRet.Append((u32CodePoint & 0x3FF) | 0xDC00);
		}
	};

	const auto pwcEnd = wsoSrc.GetEnd();
	for(auto pwcCur = wsoSrc.GetBegin(); pwcCur != pwcEnd; ++pwcCur){
		const auto wc = *pwcCur;

		switch(eState){
		case NORMAL:
			if(wc == L'\\'){
				eState = ESCAPED;
			} else {
				// eState = NORMAL;
				wcsRet.Push(wc);
			}
			break;

		case ESCAPED:
			switch(wc){
			case L'n':
				wcsRet.Append(L'\n');
				eState = NORMAL;
				break;

			case L'b':
				wcsRet.Append(L'\b');
				eState = NORMAL;
				break;

			case L'r':
				wcsRet.Append(L'\r');
				eState = NORMAL;
				break;

			case L't':
				wcsRet.Append(L'\t');
				eState = NORMAL;
				break;

			case L'\n':
				eState = NORMAL;
				break;

			case L'x':
				u32CodePoint = 0;
				uHexExpecting = 2;
				eState = UCS_CODE;
				break;

			case L'u':
				u32CodePoint = 0;
				uHexExpecting = 4;
				eState = UCS_CODE;
				break;

			case L'U':
				u32CodePoint = 0;
				uHexExpecting = 8;
				eState = UCS_CODE;
				break;

			default:
				wcsRet.Append(wc);
				eState = NORMAL;
				break;
			}
			break;

		case UCS_CODE:
			{
				auto uHex = (unsigned)(wc - L'0');
				do {
					if(uHex <= 9){
						break;
					}
					uHex += (unsigned)(L'0' - L'A');
					if(uHex <= 5){
						uHex += 0x0A;
						break;
					}
					uHex += (unsigned)(L'A' - L'a');
					if(uHex <= 5){
						uHex += 0x0A;
						break;
					}
					uHex = 0x10;
				} while(false);

				if(uHex <= 0x0F){
					u32CodePoint = (u32CodePoint << 4) | uHex;
					--uHexExpecting;
				} else {
					uHexExpecting = 0;
				}
				if(uHexExpecting != 0){
					// eState = UCS_CODE;
				} else {
					PushUtf();
					eState = NORMAL;
				}
			}
			break;
		}
	}
	if(eState == UCS_CODE){
		PushUtf();
	}

	return std::move(wcsRet);
}
void Escape(WideString &wcsAppendTo, const WideStringObserver &wsoSrc){
	const auto uSrcLength = wsoSrc.GetLength();
	wcsAppendTo.Reserve(wcsAppendTo.GetLength() + uSrcLength);

	for(std::size_t i = 0; i < uSrcLength; ++i){
		const auto wc = wsoSrc[i];

		switch(wc){
		case L'\\':
		case L'=':
		case L'{':
		case L'}':
		case L';':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(wc);
			break;

		case L' ':
			if((i == 0) || (i == uSrcLength - 1)){
				wcsAppendTo.Append(L'\\');
			}
			wcsAppendTo.Append(L' ');
			break;

		case L'\n':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L'n');
			break;

		case L'\r':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L'r');
			break;

		case L'\t':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L't');
			break;

		default:
			wcsAppendTo.Append(wc);
			break;
		}
	}
}

}

// 嵌套类定义。
struct NotationPackage::xPackages {
	struct Item {
		WideString wcsName;
		NotationPackage vPackage;
	};

	MultiIndexMap<Item,
		UniqueOrderedMemberIndex<Item, WideString, &Item::wcsName>,
		SequencedIndex<Item>> mapItems;
};
struct NotationPackage::xValues {
	struct Item {
		WideString wcsName;
		WideString wcsValue;
	};

	MultiIndexMap<Item,
		UniqueOrderedMemberIndex<Item, WideString, &Item::wcsName>,
		SequencedIndex<Item>> mapItems;
};

// ========== NotationPackage ==========
// 构造函数和析构函数。
NotationPackage::NotationPackage() noexcept {
}
NotationPackage::NotationPackage(const NotationPackage &rhs){
	*this = rhs;
}
NotationPackage::NotationPackage(NotationPackage &&rhs) noexcept {
	*this = std::move(rhs);
}
NotationPackage &NotationPackage::operator=(const NotationPackage &rhs){
	std::unique_ptr<xPackages> pPackages;
	if(rhs.xm_pPackages){
		pPackages.reset(new auto(*rhs.xm_pPackages));
	}
	std::unique_ptr<xValues> pValues;
	if(rhs.xm_pValues){
		pValues.reset(new auto(*rhs.xm_pValues));
	}
	xm_pPackages = std::move(pPackages);
	xm_pValues = std::move(pValues);
	return *this;
}
NotationPackage &NotationPackage::operator=(NotationPackage &&rhs) noexcept {
	xm_pPackages = std::move(rhs.xm_pPackages);
	xm_pValues = std::move(rhs.xm_pValues);
	return *this;
}
NotationPackage::~NotationPackage() noexcept {
}

// 其他非静态成员函数。
const NotationPackage *NotationPackage::GetPackage(const WideStringObserver &wsoName) const noexcept {
	if(!xm_pPackages){
		return nullptr;
	}
	const auto pNode = xm_pPackages->mapItems.Find<0>(wsoName);
	if(!pNode){
		return nullptr;
	}
	return &(pNode->vPackage);
}
NotationPackage *NotationPackage::GetPackage(const WideStringObserver &wsoName) noexcept {
	if(!xm_pPackages){
		return nullptr;
	}
	const auto pNode = xm_pPackages->mapItems.Find<0>(wsoName);
	if(!pNode){
		return nullptr;
	}
	return &(pNode->vPackage);
}
std::pair<NotationPackage *, bool> NotationPackage::CreatePackage(const WideStringObserver &wsoName){
	if(!xm_pPackages){
		xm_pPackages.reset(new xPackages());
	}
	std::pair<NotationPackage *, bool> vRet;
	const auto pNode = xm_pPackages->mapItems.GetLowerBound<0>(wsoName);
	if(pNode && (pNode->wcsName == wsoName)){
		vRet.first = &(pNode->vPackage);
		vRet.second = false;
	} else {
		xPackages::Item vItem;
		vItem.wcsName = WideString(wsoName);
		vRet.first = &(xm_pPackages->mapItems.InsertWithHints(
			false, std::make_tuple(pNode, nullptr), std::move(vItem)).first->vPackage);
		vRet.second = true;
	}
	return vRet;
}
std::pair<NotationPackage *, bool> NotationPackage::CreatePackage(WideString wcsName){
	if(!xm_pPackages){
		xm_pPackages.reset(new xPackages());
	}
	xPackages::Item vItem;
	vItem.wcsName = std::move(wcsName);
	const auto vResult = xm_pPackages->mapItems.Insert(false, std::move(vItem));
	return std::make_pair(&(vResult.first->vPackage), vResult.second);
}
bool NotationPackage::RemovePackage(const WideStringObserver &wsoName) noexcept {
	if(!xm_pPackages){
		return false;
	}
	const auto pNode = xm_pPackages->mapItems.Find<0>(wsoName);
	if(!pNode){
		return false;
	}
	xm_pPackages->mapItems.Erase(pNode);
	return true;
}

void NotationPackage::TraversePackages(const std::function<void (const NotationPackage &)> &fnCallback) const {
	if(!xm_pPackages){
		return;
	}
	for(auto pNode = xm_pPackages->mapItems.GetFirst<1>(); pNode; pNode = pNode->GetNext<1>()){
		fnCallback(pNode->vPackage);
	}
}
void NotationPackage::TraversePackages(const std::function<void (NotationPackage &)> &fnCallback){
	if(!xm_pPackages){
		return;
	}
	for(auto pNode = xm_pPackages->mapItems.GetFirst<1>(); pNode; pNode = pNode->GetNext<1>()){
		fnCallback(pNode->vPackage);
	}
}

const WideString *NotationPackage::GetValue(const WideStringObserver &wsoName) const noexcept {
	if(!xm_pValues){
		return nullptr;
	}
	const auto pNode = xm_pValues->mapItems.Find<0>(wsoName);
	if(!pNode){
		return nullptr;
	}
	return &(pNode->wcsValue);
}
WideString *NotationPackage::GetValue(const WideStringObserver &wsoName) noexcept {
	if(!xm_pValues){
		return nullptr;
	}
	const auto pNode = xm_pValues->mapItems.Find<0>(wsoName);
	if(!pNode){
		return nullptr;
	}
	return &(pNode->wcsValue);
}
std::pair<WideString *, bool> NotationPackage::CreateValue(const WideStringObserver &wsoName, WideString wcsValue){
	if(!xm_pValues){
		xm_pValues.reset(new xValues());
	}
	std::pair<WideString *, bool> vRet;
	const auto pNode = xm_pValues->mapItems.GetLowerBound<0>(wsoName);
	if(pNode && (pNode->wcsName == wsoName)){
		vRet.first = &(pNode->wcsValue);
		vRet.second = false;
	} else {
		xValues::Item vItem;
		vItem.wcsName = WideString(wsoName);
		vRet.first = &(xm_pValues->mapItems.InsertWithHints(
			false, std::make_tuple(pNode, nullptr), std::move(vItem)).first->wcsValue);
		vRet.second = true;
	}
	*vRet.first = std::move(wcsValue);
	return vRet;
}
std::pair<WideString *, bool> NotationPackage::CreateValue(WideString wcsName, WideString wcsValue){
	if(!xm_pValues){
		xm_pValues.reset(new xValues());
	}
	xValues::Item vItem;
	vItem.wcsName = std::move(wcsName);
	const auto vResult = xm_pValues->mapItems.Insert(false, std::move(vItem));
	vResult.first->wcsValue = std::move(wcsValue);
	return std::make_pair(&(vResult.first->wcsValue), vResult.second);
}
bool NotationPackage::RemoveValue(const WideStringObserver &wsoName) noexcept {
	if(!xm_pValues){
		return false;
	}
	const auto pNode = xm_pValues->mapItems.Find<0>(wsoName);
	if(!pNode){
		return false;
	}
	xm_pValues->mapItems.Erase(pNode);
	return true;
}

void NotationPackage::TraverseValues(const std::function<void (const WideString &)> &fnCallback) const {
	if(!xm_pValues){
		return;
	}
	for(auto pNode = xm_pValues->mapItems.GetFirst<1>(); pNode; pNode = pNode->GetNext<1>()){
		fnCallback(pNode->wcsValue);
	}
}
void NotationPackage::TraverseValues(const std::function<void (WideString &)> &fnCallback){
	if(!xm_pValues){
		return;
	}
	for(auto pNode = xm_pValues->mapItems.GetFirst<1>(); pNode; pNode = pNode->GetNext<1>()){
		fnCallback(pNode->wcsValue);
	}
}

void NotationPackage::Clear() noexcept {
	xm_pPackages.reset();
	xm_pValues.reset();
}

// ========== Notation ==========
// 其他非静态成员函数。
std::pair<Notation::ErrorType, const wchar_t *> Notation::Parse(const WideStringObserver &wsoData){
	Clear();

	auto pwcRead = wsoData.GetBegin();
	const auto pwcEnd = wsoData.GetEnd();
	if(pwcRead == pwcEnd){
		return std::make_pair(ERR_NONE, pwcRead);
	}

	VVector<Package *> vecPackageStack(1, this);

	auto pwcNameBegin = pwcRead;
	auto pwcNameEnd = pwcRead;
	auto pwcValueBegin = pwcRead;
	auto pwcValueEnd = pwcRead;

	enum {
		NAME_INDENT,
		NAME_BODY,
		NAME_PADDING,
		VAL_INDENT,
		VAL_BODY,
		VAL_PADDING,
		COMMENT,
		NAME_ESCAPED,
		VAL_ESCAPED,
		COMMENT_ESCAPED
	} eState = NAME_INDENT;

	ErrorType eError;
	const auto PushPackage = [&]{
		ASSERT(pwcNameBegin != pwcNameEnd);
		ASSERT(!vecPackageStack.IsEmpty());

		Package *ppkgSource = nullptr;
		const auto wcsSourceName = Unescape(WideStringObserver(pwcValueBegin, pwcValueEnd));
		if(!wcsSourceName.IsEmpty()){
			ppkgSource = vecPackageStack.GetEnd()[-1]->GetPackage(wcsSourceName);
			if(!ppkgSource){
				eError = ERR_SOURCE_PACKAGE_NOT_FOUND;
				return false;
			}
		}

		const auto vResult = vecPackageStack.GetEnd()[-1]->CreatePackage(
			Unescape(WideStringObserver(pwcNameBegin, pwcNameEnd))
		);
		if(!vResult.second){
			eError = ERR_DUPLICATE_PACKAGE;
			return false;
		}
		if(ppkgSource){
			*vResult.first = *ppkgSource;
		}
		vecPackageStack.Push(vResult.first);
		return true;
	};
	const auto PopPackage = [&]{
		if(vecPackageStack.GetSize() <= 1){
			eError = ERR_UNEXCEPTED_PACKAGE_CLOSE;
			return false;
		}
		vecPackageStack.Pop();
		return true;
	};
	const auto SubmitValue = [&]{
		ASSERT(pwcNameBegin != pwcNameEnd);
		ASSERT(!vecPackageStack.IsEmpty());

		const auto vResult = vecPackageStack.GetEnd()[-1]->CreateValue(
			Unescape(WideStringObserver(pwcNameBegin, pwcNameEnd)),
			Unescape(WideStringObserver(pwcValueBegin, pwcValueEnd))
		);
		if(!vResult.second){
			eError = ERR_DUPLICATE_VALUE;
			return false;
		}
		return true;
	};

	do {
		const wchar_t wc = *pwcRead;

		switch(wc){
		case L'=':
			switch(eState){
			case NAME_INDENT:
				return std::make_pair(ERR_NO_VALUE_NAME, pwcRead);

			case NAME_BODY:
			case NAME_PADDING:
				eState = VAL_INDENT;
				continue;

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
			case COMMENT:
			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L'{':
			switch(eState){
			case NAME_INDENT:
				return std::make_pair(ERR_NO_VALUE_NAME, pwcRead);

			case NAME_BODY:
			case NAME_PADDING:
				if(!PushPackage()){
					return std::make_pair(eError, pwcRead);
				}
				eState = NAME_INDENT;
				continue;

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
				if(!PushPackage()){
					return std::make_pair(eError, pwcRead);
				}
				eState = NAME_INDENT;
				continue;

			case COMMENT:
			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L'}':
			switch(eState){
			case NAME_INDENT:
				if(!PopPackage()){
					return std::make_pair(eError, pwcRead);
				}
				// eState = NAME_INDENT;
				continue;

			case NAME_BODY:
			case NAME_PADDING:
				return std::make_pair(ERR_EQU_EXPECTED, pwcRead);

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
				if(!SubmitValue() || !PopPackage()){
					return std::make_pair(eError, pwcRead);
				}
				eState = NAME_INDENT;
				continue;

			case COMMENT:
			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L';':
			switch(eState){
			case NAME_INDENT:
				eState = COMMENT;
				continue;

			case NAME_BODY:
			case NAME_PADDING:
				return std::make_pair(ERR_EQU_EXPECTED, pwcRead);

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
				if(!SubmitValue()){
					return std::make_pair(eError, pwcRead);
				}
				eState = COMMENT;
				continue;

			case COMMENT:
			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L'\n':
			switch(eState){
			case NAME_INDENT:
				continue;

			case NAME_BODY:
			case NAME_PADDING:
				return std::make_pair(ERR_EQU_EXPECTED, pwcRead);

			case VAL_INDENT:
			case VAL_BODY:
			case VAL_PADDING:
				if(!SubmitValue()){
					return std::make_pair(eError, pwcRead);
				}
				eState = NAME_INDENT;
				continue;

			case COMMENT:
				eState = NAME_INDENT;
				continue;

			case NAME_ESCAPED:
			case VAL_ESCAPED:
			case COMMENT_ESCAPED:
				break;
			};
			break;
		}

		switch(eState){
		case NAME_INDENT:
			switch(wc){
			case L' ':
			case L'\t':
				// eState = NAME_INDENT;
				break;

			default:
				pwcNameBegin = pwcRead;
				pwcNameEnd = pwcRead + 1;
				pwcValueBegin = pwcRead;
				pwcValueEnd = pwcRead;
				eState = (wc == L'\\') ? NAME_ESCAPED : NAME_BODY;
				break;
			}
			break;

		case NAME_BODY:
			switch(wc){
			case L' ':
			case L'\t':
				eState = NAME_PADDING;
				break;

			default:
				pwcNameEnd = pwcRead + 1;
				if(wc == L'\\'){
					eState = NAME_ESCAPED;
				}
				break;
			}
			break;

		case NAME_PADDING:
			switch(wc){
			case L' ':
			case L'\t':
				// eState = NAME_PADDING;
				break;

			default:
				pwcNameEnd = pwcRead + 1;
				eState = (wc == L'\\') ? NAME_ESCAPED : NAME_BODY;
				break;
			}
			break;

		case VAL_INDENT:
			switch(wc){
			case L' ':
			case L'\t':
				// eState = VAL_INDENT;
				break;

			default:
				pwcValueBegin = pwcRead;
				pwcValueEnd = pwcRead + 1;
				eState = (wc == L'\\') ? VAL_ESCAPED : VAL_BODY;
				break;
			}
			break;

		case VAL_BODY:
			switch(wc){
			case L' ':
			case L'\t':
				eState = VAL_PADDING;
				break;

			default:
				pwcValueEnd = pwcRead + 1;
				if(wc == L'\\'){
					eState = VAL_ESCAPED;
				}
				break;
			}
			break;

		case VAL_PADDING:
			switch(wc){
			case L' ':
			case L'\t':
				// eState = VAL_PADDING;
				break;

			default:
				pwcValueEnd = pwcRead + 1;
				eState = (wc == L'\\') ? VAL_ESCAPED : VAL_BODY;
				break;
			}
			break;

		case COMMENT:
			if(wc == L'\\'){
				eState = COMMENT_ESCAPED;
			}
			break;

		case NAME_ESCAPED:
			pwcNameEnd = pwcRead + 1;
			eState = NAME_BODY;
			break;

		case VAL_ESCAPED:
			pwcValueEnd = pwcRead + 1;
			eState = VAL_BODY;
			break;

		case COMMENT_ESCAPED:
			eState = COMMENT;
			break;
		}
	} while(++pwcRead != pwcEnd);

	switch(eState){
	case NAME_INDENT:
		break;

	case NAME_BODY:
	case NAME_PADDING:
	case NAME_ESCAPED:
		return std::make_pair(ERR_EQU_EXPECTED, pwcRead);

	case VAL_INDENT:
	case VAL_BODY:
	case VAL_PADDING:
	case VAL_ESCAPED:
		if(!SubmitValue()){
			return std::make_pair(eError, pwcRead);
		}
		break;

	case COMMENT:
	case COMMENT_ESCAPED:
		break;
	};
	if(vecPackageStack.GetSize() > 1){
		return std::make_pair(ERR_UNCLOSED_PACKAGE, pwcRead);
	}

	return std::make_pair(ERR_NONE, pwcRead);
}
WideString Notation::Export(const WideStringObserver &wsoIndent) const {
	WideString wcsRet;

	VVector<std::pair<const Package *,
		decltype(xm_pPackages->mapItems.GetFirst<1>())>
		> vecPackageStack;
	vecPackageStack.Push(this, xm_pPackages ? xm_pPackages->mapItems.GetFirst<1>() : nullptr);

	WideString wcsIndent;
	for(;;){
		auto &vTop = vecPackageStack.GetEnd()[-1];

		if(vTop.second){
			wcsRet.Append(wcsIndent);
			Escape(wcsRet, vTop.second->wcsName);
			wcsRet.Append(L' ');
			wcsRet.Append(L'{');
			wcsRet.Append(L'\n');
			wcsIndent.Append(wsoIndent);
			vecPackageStack.Push(&(vTop.second->vPackage),
				vTop.second->vPackage.xm_pPackages
					? vTop.second->vPackage.xm_pPackages->mapItems.GetFirst<1>()
					: nullptr);
			vTop.second = vTop.second->GetNext<1>();
			continue;
		}

		if(vTop.first->xm_pValues){
			for(auto pNode = vTop.first->xm_pValues->mapItems.GetFirst<1>();
				pNode; pNode = pNode->GetNext<1>())
			{
				wcsRet.Append(wcsIndent);
				Escape(wcsRet, pNode->wcsName);
				wcsRet.Append(L' ');
				wcsRet.Append(L'=');
				wcsRet.Append(L' ');
				Escape(wcsRet, pNode->wcsValue);
				wcsRet.Append(L'\n');
			}
		}

		vecPackageStack.Pop();
		if(vecPackageStack.IsEmpty()){
			break;
		}

		wcsIndent.Truncate(wsoIndent.GetLength());
		wcsRet.Append(wcsIndent);
		wcsRet.Append(L'}');
		wcsRet.Append(L'\n');
	}

	ASSERT(wcsIndent.IsEmpty());

	return std::move(wcsRet);
}
