// Copyleft 2013, LH_Mouse. All wrongs reserved.

#ifndef __DEPENDENCY_DATABASE_HPP__
#define __DEPENDENCY_DATABASE_HPP__

#include <set>
#include <string>
#include <map>

namespace MCFBuild {
	class DependencyDatabase {
	public:
		struct Dependencies {
			long long m_llTimestamp;
			std::set<std::wstring> m_setDependencyFiles;
		};

		typedef std::map<std::wstring, Dependencies> DEPENDENCY_MAP;
	private:
		DEPENDENCY_MAP xm_mapDependencies;
	public:
		const DEPENDENCY_MAP &GetRawMap() const {
			return xm_mapDependencies;
		}
		DEPENDENCY_MAP &GetRawMap(){
			return xm_mapDependencies;
		}
	public:
		void SaveToFile(const std::wstring &wcsPath) const;
		void LoadFromFile(const std::wstring &wcsPath);
	};
}

#endif
