// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include <algorithm>
#include <shlwapi.h>
#include "RAIIWrapper.hpp"
using namespace MCFBuild;

namespace {
	void GetTreeRecur(
		FOLDER_TREE &ret,
		std::size_t &uTotal,
		const std::wstring &wcsPath,
		const PROJECT &Project,
		const std::wstring &wcsPrefix,
		bool bVerbose
	){
		Output(wcsPrefix + L"正在统计目录“" + wcsPath + L"”...");

		struct FindCloser {
			constexpr HANDLE Null(){
				return INVALID_HANDLE_VALUE;
			}
			void Close(HANDLE hObj){
				::FindClose(hObj);
			}
		};

		unsigned long ulFileCount = 0;
		WIN32_FIND_DATAW FindData;
		const auto hFindFile = MCF::UniqueHandle<HANDLE, FindCloser>(::FindFirstFileW((wcsPath + L"*").c_str(), &FindData));
		if(hFindFile.IsGood()){
			do {
				if((__builtin_memcmp(FindData.cFileName, L".", sizeof(L".")) == 0) || (__builtin_memcmp(FindData.cFileName, L"..", sizeof(L"..")) == 0)){
					continue;
				}
				if(std::find_if(
					Project.setIgnoredFiles.cbegin(),
					Project.setIgnoredFiles.cend(),
					[&FindData](const std::wstring &wcsExpression) -> bool {
						return ::PathMatchSpecW(FindData.cFileName, wcsExpression.c_str()) != FALSE;
					}
				) != Project.setIgnoredFiles.end()){
					continue;
				}

				std::wstring wcsName(FindData.cFileName);
				if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
					++ulFileCount;
					__builtin_memcpy(&(ret.mapFiles[std::move(wcsName)]), &FindData.ftLastWriteTime, sizeof(long long));
				} else {
					const std::wstring wcsNextPath(wcsPath + wcsName + L'\\');
					GetTreeRecur(ret.mapSubFolders[std::move(wcsName)], uTotal, wcsNextPath, Project, wcsPrefix + L"  ", bVerbose);
				}
			} while(::FindNextFileW(hFindFile, &FindData) != FALSE);

			if(bVerbose){
				Output(L"%ls  目录“%ls”中共有 %lu 个文件。", wcsPrefix.c_str(), wcsPath.c_str(), ulFileCount);
			}
			uTotal += ulFileCount;
		} else {
			if(bVerbose){
				Error(wcsPrefix + L"  打开目录“" + wcsPath + L"”失败。");
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
		GetTreeRecur(ret, uTotal, wcsPath, Project, L"  ", bVerbose);
		Output(L"  共发现 %lu 个文件。", uTotal);

		return std::move(ret);
	}
}
