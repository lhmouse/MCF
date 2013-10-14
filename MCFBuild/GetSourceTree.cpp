// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include <algorithm>
#include <shlwapi.h>
#include "RAIIWrapperTemplate.hpp"
using namespace MCFBuild;

namespace {
	void GetTreeRecur(
		FOLDER_TREE &ret,
		std::size_t &uTotal,
		const std::wstring &wcsPath,
		const PROJECT &Project,
		bool bVerbose
	){
		Output(L"  正在统计目录“" + wcsPath + L"”...");

		struct FindCloser {
			constexpr static HANDLE Null(){ return INVALID_HANDLE_VALUE; }
			void operator()(HANDLE hObj) const { ::FindClose(hObj); }
		};

		unsigned long ulFileCount = 0;
		WIN32_FIND_DATAW FindData;
		const auto hFindFile = MCF::RAIIWrapper<HANDLE, FindCloser>(::FindFirstFileW((wcsPath + L"*").c_str(), &FindData));
		if(hFindFile.IsGood()){
			do {
				if((__builtin_memcmp(FindData.cFileName, L".", sizeof(L".")) == 0) || (__builtin_memcmp(FindData.cFileName, L"..", sizeof(L"..")) == 0)){
					continue;
				}
				if(std::find_if(
					Project.setIgnored.cbegin(),
					Project.setIgnored.cend(),
					[&FindData](const std::wstring &wcsExpression) -> bool {
						return ::PathMatchSpecW(FindData.cFileName, wcsExpression.c_str()) != FALSE;
					}
				) != Project.setIgnored.end()){
					continue;
				}

				std::wstring wcsName(FindData.cFileName);
				if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
					++ulFileCount;
					__builtin_memcpy(&(ret.mapFiles[std::move(wcsName)]), &FindData.ftLastWriteTime, sizeof(long long));
				} else {
					GetTreeRecur(ret.mapSubFolders[std::move(wcsName)], uTotal, wcsPath + wcsName + L'\\', Project, bVerbose);
				}
			} while(::FindNextFileW(hFindFile, &FindData) != FALSE);

			if(bVerbose){
				Output(L"    目录“%ls”中共有 %lu 个文件。", wcsPath.c_str(), ulFileCount);
			}
			uTotal += ulFileCount;
		} else {
			if(bVerbose){
				Error(L"    打开目录“" + wcsPath + L"”失败。");
			}
		}
	}
}

namespace MCFBuild {
	FOLDER_TREE GetSourceTree(
		const std::wstring &wcsPath,
		const PROJECT &Project,
		bool bVerbose
	){
		FOLDER_TREE ret;

		std::size_t uTotal = 0;
		GetTreeRecur(ret, uTotal, wcsPath, Project, bVerbose);
		Output(L"  共发现 %lu 个文件。", uTotal);

		return std::move(ret);
	}
}
