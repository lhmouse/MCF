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
	private:
		std::map<std::wstring, Dependencies> xm_mapDependencies;
	public:
		void SaveToFile(const std::wstring &wcsPath) const;
		void LoadFromFile(const std::wstring &wcsPath);

		const Dependencies *Get(const std::wstring &wcsKey) const;
		Dependencies &Add(std::wstring &&wcsKey);
		bool Remove(const std::wstring &wcsKey);
	};
}

#endif
