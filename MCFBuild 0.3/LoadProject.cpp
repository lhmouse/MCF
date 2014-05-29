// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>
#include <memory>
#include <windows.h>
#include <shlwapi.h>
#include "UniqueHandle.hpp"
#include "NotationClass.hpp"
using namespace MCFBuild;

namespace {
	struct PACKAGEW {
		std::map<wvstring, PACKAGEW> mapPackages;
		std::map<wvstring, wvstring> mapValues;
	};

	vstring GetProjFileContents(long long *pllTimestamp, const wvstring &wcsProjFile){
		struct FileCloser {
			constexpr HANDLE operator()(){
				return INVALID_HANDLE_VALUE;
			}
			void operator()(HANDLE hObj){
				::CloseHandle(hObj);
			}
		};

		const auto hProjFile = MCF::UniqueHandle<HANDLE, FileCloser>(::CreateFileW(wcsProjFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL));
		if(!hProjFile.IsGood()){
			const DWORD dwError = ::GetLastError();
			throw Exception{dwError, L"打开项目文件“" + wcsProjFile + L"”失败。"};
		}

		LARGE_INTEGER liFileSize;
		::GetFileSizeEx(hProjFile, &liFileSize);
		if(liFileSize.QuadPart == 0){
			throw Exception{ERROR_INVALID_DATA, L"文件“" + wcsProjFile + L"”为空。"};
		} else if(liFileSize.QuadPart >= 16 * 0x400 * 0x400){
			throw Exception{ERROR_INVALID_DATA, L"文件“" + wcsProjFile + L"”太大。"};
		}

		const std::size_t uFileSize = (std::size_t)liFileSize.QuadPart;
		std::unique_ptr<char[]> pchContents(new char[uFileSize]);

		std::size_t uBytesTotal = 0;
		DWORD dwBytesRead;
		do {
			if(::ReadFile(hProjFile, pchContents.get() + uBytesTotal, (DWORD)(uFileSize - uBytesTotal), &dwBytesRead, nullptr) == FALSE){
				const DWORD dwError = ::GetLastError();
				throw Exception{dwError, L"读取项目文件“" + wcsProjFile + L"”时出错。"};
			}
			uBytesTotal += dwBytesRead;
		} while(dwBytesRead != 0);

		if(pllTimestamp != nullptr){
			union {
				FILETIME ft;
				long long ll;
			} u;

			if(::GetFileTime(hProjFile, nullptr, nullptr, &u.ft) != FALSE){
				*pllTimestamp = u.ll;
			}
		}

		return vstring(pchContents.get(), pchContents.get() + (std::ptrdiff_t)uFileSize);
	}

	MCF::NotationClass ParseProject(vstring &&strContents){
		auto iterWrite = strContents.begin();
		auto iterRead = strContents.cbegin();
		std::vector<const char *> vecCRLFBreaks;
		char chNext = *iterRead;
		do {
			const char chCur = chNext;
			chNext = *++iterRead;
			if(chCur == '\r'){
				if(chNext == '\n'){
					vecCRLFBreaks.push_back(&*iterWrite);
					chNext = *++iterRead;
				}
				*(iterWrite++) = '\n';
			} else {
				*(iterWrite++) = chCur;
			}
		} while(chNext != 0);
		strContents.erase(iterWrite, strContents.end());

		MCF::NotationClass Project;
		const auto ParseError = Project.Parse(strContents.c_str(), strContents.size());
		if(ParseError.first != MCF::NotationClass::ERR_NONE){
			static const wchar_t *PARSE_ERROR_TABLE[] = {
				L"ERR_UNKNOWN",
				L"ERR_NO_VALUE_NAME",
				L"ERR_NO_PACKAGE_NAME",
				L"ERR_UNEXCEPTED_PACKAGE_CLOSE",
				L"ERR_EQU_EXPECTED",
				L"ERR_UNCLOSED_PACKAGE",
				L"ERR_ESCAPE_AT_EOF"
			};
			wchar_t awchInfo[0x40];
			std::swprintf(
				awchInfo,
				COUNTOF(awchInfo),
				L"在第 %lu 个字节处发生错误 %ls。",
				(unsigned long)((ParseError.second - strContents.c_str()) +(std::lower_bound(vecCRLFBreaks.cbegin(), vecCRLFBreaks.cend(), ParseError.second) - vecCRLFBreaks.cbegin())),
				((std::size_t)ParseError.first < COUNTOF(PARSE_ERROR_TABLE)) ? PARSE_ERROR_TABLE[(std::size_t)ParseError.first] : PARSE_ERROR_TABLE[0]
			);
			throw Exception{ERROR_INVALID_DATA, wvstring(L"解析项目文件时出错，") + awchInfo};
		}
		return std::move(Project);
	}

	void LoadPackages(
		std::vector<std::pair<wvstring, PACKAGEW>> &vecPackages,
		const MCF::NotationClass &Project,
		const wvstring &wcsID,
		const wvstring &wcsPrefix,
		bool bVerbose
	){
		Output(wcsPrefix + L"正在加载配置包“" + wcsID + L"”...");

		const auto u8sID(WcsToU8s(wcsID));

		const MCF::NotationClass::Package *const pPackage = Project.OpenPackage(u8sID.c_str());

		std::vector<wvstring> vecInheritedFrom;
		const std::string *const pu8sInheritedFrom = Project.GetValue(nullptr, u8sID.c_str());
		if(pu8sInheritedFrom != nullptr){
			const auto wcsInheritedFrom(U8sToWcs(*pu8sInheritedFrom));
			std::size_t uPos = 0;
			do {
				auto uBegin = uPos;
				auto uEnd = wcsInheritedFrom.find(L',', uPos + 1);
				if(uEnd == wvstring::npos){
					uEnd = wcsInheritedFrom.size();
					uPos = wvstring::npos;
				} else {
					uPos = uEnd + 1;
				}
				while(uBegin != uEnd){
					if((wcsInheritedFrom[uBegin] == L' ') || (wcsInheritedFrom[uBegin] == L'\t')){
						++uBegin;
					} else if((wcsInheritedFrom[uEnd - 1] == L' ') || (wcsInheritedFrom[uEnd - 1] == L'\t')){
						--uEnd;
					} else {
						break;
					}
				}
				if(uBegin == uEnd){
					Error(wcsPrefix + L"配置包“" + wcsID + L"”指定了一个空的继承包，已忽略。");
				} else {
					auto wcsSingleInheritedFrom(wcsInheritedFrom.substr(uBegin, uEnd));
					if(wcsSingleInheritedFrom == wcsID){
						throw Exception{ERROR_INVALID_DATA, L"配置包“" + wcsID + L"”指定了自身作为继承包。"};
					}
					vecInheritedFrom.emplace_back(std::move(wcsSingleInheritedFrom));
				}
			} while(uPos != wvstring::npos);
		}

		if(vecInheritedFrom.empty()){
			if(pPackage == nullptr){
				throw Exception{ERROR_INVALID_DATA, L"配置包“" + wcsID + L"”没有定义。"};
			}
		}

		for(const auto &pkgHistory : vecPackages){
			if(pkgHistory.first == wcsID){
				throw Exception{ERROR_INVALID_DATA, L"加载配置包“" + wcsID + L"”时侦测到重复继承。"};
			}
		}
		vecPackages.emplace_back(std::move(wcsID), PACKAGEW());

		if(pPackage != nullptr){
			struct Wrapper {
				static void LoadPackage(PACKAGEW &pkgDst, const MCF::NotationClass::Package &pkgSrc){
					for(const auto &val : pkgSrc.mapPackages){
						LoadPackage(pkgDst.mapPackages[U8sToWcs(val.first)], val.second);
					}
					for(const auto &val : pkgSrc.mapValues){
						pkgDst.mapValues[U8sToWcs(val.first)] = U8sToWcs(val.second);
					}
				}
			};

			Wrapper::LoadPackage(vecPackages.back().second, *pPackage);
		}

		while(!vecInheritedFrom.empty()){
			LoadPackages(vecPackages, Project, vecInheritedFrom.back(), wcsPrefix + L"  ", bVerbose);
			vecInheritedFrom.pop_back();
		}
	}

	void MergePackage(PACKAGEW &pkgTo, PACKAGEW &&pkgFrom){
		const auto MergeString = [](wvstring &wcsTo, wvstring &&wcsFrom) -> void {
			if(wcsFrom.empty()){
				return;
			}

			enum {
				INSERT_BEFORE,
				INSERT_AFTER,
				REPLACE,
				DEFAULT
			} eMode;

			auto iterBegin = wcsFrom.cbegin();

			switch(*iterBegin){
			case L'<':
				eMode = INSERT_BEFORE;
				break;
			case L'>':
				eMode = INSERT_AFTER;
				break;
			case L'=':
				eMode = REPLACE;
				break;
			default:
				eMode = DEFAULT;
				break;
			}
			if(eMode != DEFAULT){
				wchar_t ch;
				do {
					ch = *++iterBegin;
				} while((ch == L' ') || (ch == L'\t'));
			}

			const std::size_t uFromLen = (std::size_t)(wcsFrom.cend() - iterBegin);
			switch(eMode){
			case INSERT_BEFORE:
				{
					const std::size_t uOldSize = wcsTo.size();
					wcsTo.resize(uOldSize + uFromLen + 1);
					std::copy_backward(wcsTo.begin(), wcsTo.begin() + uOldSize, wcsTo.end());
					std::copy(iterBegin, wcsFrom.cend(), wcsTo.begin());
					wcsTo[uFromLen] = L' ';
				}
				break;
			case INSERT_AFTER:
				{
					const std::size_t uOldSize = wcsTo.size();
					wcsTo.resize(uOldSize + uFromLen + 1);
					std::copy(iterBegin, wcsFrom.cend(), wcsTo.begin() + uOldSize + 1);
					wcsTo[uOldSize] = L' ';
				}
				break;
			default:
				wcsTo = std::move(wcsFrom);
				break;
			}
		};

		for(auto &val : pkgFrom.mapPackages){
			MergePackage(pkgTo.mapPackages[val.first], std::move(val.second));
		}
		for(auto &val : pkgFrom.mapValues){
			MergeString(pkgTo.mapValues[val.first], std::move(val.second));
		}
	}

	void ExpandValue(wvstring &wcsToExpand, const std::map<wvstring, wvstring> &mapValues, const wchar_t *pwszSelfID){
		struct Wrapper {
			static void ExpandString(
				wvstring &wcsToExpand,
				const std::map<wvstring, wvstring> &mapValues,
				std::vector<wvstring> &vecHistory,
				std::set<wvstring> &setWarned
			){
				wvstring wcsTemp;
				wcsTemp.reserve(wcsToExpand.size());

				auto iterRead = wcsToExpand.cbegin();
				while(iterRead != wcsToExpand.cend()){
					const wchar_t ch = *(iterRead++);
					if(ch != L'$'){
						wcsTemp.push_back(ch);
						continue;
					}

					auto iterNameEnd = iterRead;
					for(;;){
						if(iterNameEnd == wcsToExpand.cend()){
							throw Exception{ERROR_INVALID_DATA, L"“" + wcsToExpand + L"”中的 $ 不匹配。"};
						}
						if(*iterNameEnd == L'$'){
							break;
						}
						++iterNameEnd;
					}
					if(iterNameEnd == iterRead){
						wcsTemp.push_back(L'$');
						++iterRead;
						continue;
					}

					wvstring wcsVarName(iterRead, iterNameEnd);
					iterRead = ++iterNameEnd;

					const auto iterHistory = std::find(vecHistory.cbegin(), vecHistory.cend(), wcsVarName);
					if(iterHistory != vecHistory.cend()){
						if(setWarned.find(wcsVarName) == setWarned.end()){
							Error(L"  警告：侦测到变量“" + wcsVarName + L"”的递归展开，已替换为空字符串。");
							setWarned.insert(std::move(wcsVarName));
						}
						continue;
					}
					const auto iterVar = mapValues.find(wcsVarName);
					if(iterVar == mapValues.end()){
						Error(L"  警告：变量“" + wcsVarName + L"”没有定义，已替换为空字符串。");
						continue;
					}

					wvstring wcsExpanded(iterVar->second);
					vecHistory.push_back(std::move(wcsVarName));
					ExpandString(wcsExpanded, mapValues, vecHistory, setWarned);
					vecHistory.pop_back();
					wcsTemp.append(wcsExpanded);
				}
				wcsToExpand = std::move(wcsTemp);
			}
		};

		std::vector<wvstring> vecHistory;
		std::set<wvstring> setWarned;
		if(pwszSelfID != nullptr){
			vecHistory.push_back(pwszSelfID);
		}
		Wrapper::ExpandString(wcsToExpand, mapValues, vecHistory, setWarned);
		if(pwszSelfID != nullptr){
			vecHistory.pop_back();
		}
		assert(vecHistory.empty());
	}

	template<typename... T>
	inline const PACKAGEW *DoGetPackage(
		const PACKAGEW &pkgCurrent,
		bool bRequired,
		wvstring &wcsHistory,
		const wchar_t **ppwszLast,
		const wchar_t *pwszNext,
		const T... PathParts
	){
		wcsHistory.append(pwszNext);

		const auto iter = pkgCurrent.mapPackages.find(pwszNext);
		if(iter == pkgCurrent.mapPackages.end()){
			if(bRequired){
				throw Exception{ERROR_INVALID_DATA, L"配置包“" + wcsHistory + L"”未定义。"};
			}
			return nullptr;
		}

		wcsHistory.push_back(L'/');
		if(ppwszLast != nullptr){
			*ppwszLast = pwszNext;
		}

		return DoGetPackage(iter->second, bRequired, wcsHistory, ppwszLast, static_cast<const wchar_t *>(PathParts)...);
	}
	template<>
	inline const PACKAGEW *DoGetPackage(
		const PACKAGEW &pkgCurrent,
		bool,
		wvstring &wcsHistory,
		const wchar_t **ppwszLast,
		const wchar_t *pwszNext
	){
		if(pwszNext != nullptr){
			wcsHistory.append(pwszNext);
			if(ppwszLast != nullptr){
				*ppwszLast = pwszNext;
			}
		}

		return &pkgCurrent;
	}

	template<typename... T>
	const PACKAGEW *GetPackage(
		const PACKAGEW &pkgTop,
		const wchar_t *pwszPrefix,
		bool bRequired,
		T... PathParts
	){
		wvstring wcsHistory;
		if(pwszPrefix != nullptr){
			wcsHistory.assign(pwszPrefix);
		}
		return DoGetPackage(
			pkgTop,
			bRequired,
			wcsHistory,
			nullptr,
			static_cast<const wchar_t *>(PathParts)...,
			nullptr
		);
	}

	template<typename P, typename... T>
	wvstring GetExpandedValue(
		const PACKAGEW &pkgTop,
		const PACKAGEW &pkgExpandFrom,
		const wchar_t *pwszPrefix,
		bool bRequired,
		P Placeholder,
		T... PathParts
	){
		wvstring wcsHistory;
		if(pwszPrefix != nullptr){
			wcsHistory.assign(pwszPrefix);
		}
		const wchar_t *pwszID = nullptr;
		const PACKAGEW *const ppkgInWhich = DoGetPackage(
			pkgTop,
			bRequired,
			wcsHistory,
			&pwszID,
			static_cast<const wchar_t *>(Placeholder),
			static_cast<const wchar_t *>(PathParts)...
		);
		if(ppkgInWhich != nullptr){
			const auto iter = ppkgInWhich->mapValues.find(pwszID);
			if(iter != ppkgInWhich->mapValues.end()){
				wvstring wcsRet(iter->second);
				ExpandValue(wcsRet, pkgExpandFrom.mapValues, (ppkgInWhich == &pkgExpandFrom) ? pwszID : nullptr);
				return std::move(wcsRet);
			}
		}
		if(bRequired){
			throw Exception{ERROR_INVALID_DATA, L"值“" + wcsHistory + L"”未定义。"};
		}
		return wvstring();
	}
	template<typename P, typename... T>
	wvstring GetExpandedValue(
		const PACKAGEW &pkgTop,
		const wchar_t *pwszPrefix,
		bool bRequired,
		P Placeholder,
		T... PathParts
	){
		return GetExpandedValue(pkgTop, pkgTop, pwszPrefix, bRequired, Placeholder, PathParts...);
	}
}

namespace MCFBuild {
	PROJECT LoadProject(
		const wvstring &wcsProjFile,
		const wvstring &wcsConfig,
		const wvstring &wcsSrcRoot,
		const wvstring &wcsDstRoot,
		const std::map<wvstring, wvstring> &mapVars,
		const wvstring &wcsOutputPath,
		bool bVerbose
	){
		PROJECT ret;

		std::vector<std::pair<wvstring, PACKAGEW>> vecPackages;
		LoadPackages(vecPackages, ParseProject(GetProjFileContents(&ret.llProjectFileTimestamp, wcsProjFile)), wcsConfig, L"  ", bVerbose);

		PACKAGEW pkgTop;
		while(!vecPackages.empty()){
			MergePackage(pkgTop, std::move(vecPackages.back().second));
			vecPackages.pop_back();
		}

		wvstring wcsCurrentDir(L".");
		FixPath(wcsCurrentDir);

		PACKAGEW pkgFinal;
		pkgFinal.mapValues = mapVars;
		pkgFinal.mapValues[L"_CD"]		= std::move(wcsCurrentDir);
		pkgFinal.mapValues[L"_CONFIG"]	= wcsConfig;
		pkgFinal.mapValues[L"_SRCDIR"]	= wcsSrcRoot;
		pkgFinal.mapValues[L"_OBJDIR"]	= wcsDstRoot;
		MergePackage(pkgTop, std::move(pkgFinal));

		auto wcsRawIgnoredFiles = GetExpandedValue(pkgTop, nullptr, false, L"IgnoredFiles");
		if(!wcsRawIgnoredFiles.empty()){
			auto pwszNextTok = &wcsRawIgnoredFiles[0];
			do {
				const auto pwszTok = pwszNextTok;
				pwszNextTok = ::PathGetArgsW(pwszNextTok);

				::PathRemoveArgsW(pwszTok);
				::PathUnquoteSpacesW(pwszTok);
				if(pwszTok[0] != 0){
					ret.setIgnoredFiles.emplace(pwszTok);
				}
			} while(pwszNextTok[0] != 0);

			if(bVerbose){
				Output(L"  被忽略的文件或目录：");
				for(const auto &wcsIgnoredFile : ret.setIgnoredFiles){
					Output(L"    " + wcsIgnoredFile);
				}
			}
		}

		const auto pmapRawPreCompiledHeaders = GetPackage(pkgTop, nullptr, false, L"PreCompiledHeaders");
		if(pmapRawPreCompiledHeaders != nullptr){
			for(const auto &RawPreCompiledHeaderItem : pmapRawPreCompiledHeaders->mapPackages){
				const wvstring wcsPrefix(L"PreCompiledHeaders/" + RawPreCompiledHeaderItem.first + L'/');
				auto wcsSourceFile = GetExpandedValue(RawPreCompiledHeaderItem.second, pkgTop, wcsPrefix.c_str(), true, L"SourceFile");
				auto wcsCommandLine = GetExpandedValue(RawPreCompiledHeaderItem.second, pkgTop, wcsPrefix.c_str(), true, L"CommandLine");

				auto &PreCompiledHeader = ret.mapPreCompiledHeaders[RawPreCompiledHeaderItem.first];
				PreCompiledHeader.wcsSourceFile		= std::move(wcsSourceFile);
				PreCompiledHeader.wcsCommandLine	= std::move(wcsCommandLine);
			}
			if(bVerbose){
				Output(L"  预编译头：");
				for(const auto &PreCompiledHeaderItem : ret.mapPreCompiledHeaders){
					Output(L"    名称“" + PreCompiledHeaderItem.first + L"”：");
					Output(L"      源文件：" + PreCompiledHeaderItem.second.wcsSourceFile);
					Output(L"      命令行：" + PreCompiledHeaderItem.second.wcsCommandLine);
				}
			}
		}

		const auto &mapRawCompilers = GetPackage(pkgTop, nullptr, true, L"Compilers")->mapPackages;
		for(const auto &RawCompilerItem : mapRawCompilers){
			const wvstring wcsPrefix(L"Compilers/" + RawCompilerItem.first + L'/');
			const auto wcsCommandLine = GetExpandedValue(RawCompilerItem.second, pkgTop, wcsPrefix.c_str(), true, L"CommandLine");
			const auto wcsDependency = GetExpandedValue(RawCompilerItem.second, pkgTop, wcsPrefix.c_str(), false, L"Dependency");

			auto wcsRawExtensions = RawCompilerItem.first;
			auto pwszNextTok = &wcsRawExtensions[0];
			do {
				const auto pwszTok = pwszNextTok;
				pwszNextTok = ::PathGetArgsW(pwszNextTok);

				::PathRemoveArgsW(pwszTok);
				::PathUnquoteSpacesW(pwszTok);
				if(pwszTok[0] != 0){
					auto &Compiler = ret.mapCompilers[&pwszTok[0]];
					Compiler.wcsCommandLine = wcsCommandLine;
					Compiler.wcsDependency = wcsDependency;
				}
			} while(pwszNextTok[0] != 0);
		}
		if(bVerbose){
			Output(L"  编译器：");
			for(const auto &CompilerItem : ret.mapCompilers){
				Output(L"    扩展名“" + CompilerItem.first + L"”：");
				Output(L"      命令行　　：" + CompilerItem.second.wcsCommandLine);
				if(CompilerItem.second.wcsDependency.empty()){
					Output(L"      依赖检查器：<无依赖关系>");
				} else {
					Output(L"      依赖检查器：" + CompilerItem.second.wcsDependency);
				}
			}
		}

		ret.Linkers.wcsPartial = GetExpandedValue(pkgTop, nullptr, true, L"Linkers", L"Partial");
		ret.Linkers.wcsFull = GetExpandedValue(pkgTop, nullptr, true, L"Linkers", L"Full");
		if(bVerbose){
			Output(L"  链接器：");
			Output(L"    部分链接器：" + ret.Linkers.wcsPartial);
			Output(L"    完全链接器：" + ret.Linkers.wcsFull);
		}

		ret.wcsOutputPath = wcsOutputPath;
		if(ret.wcsOutputPath.back() == L'\\'){
			ret.wcsOutputPath.append(GetExpandedValue(pkgTop, nullptr, true, L"DefaultOutput"));
			FixPath(ret.wcsOutputPath);
		}
		Output(L"  输出文件：");
		Output(L"    " + ret.wcsOutputPath);

		return std::move(ret);
	}
}
