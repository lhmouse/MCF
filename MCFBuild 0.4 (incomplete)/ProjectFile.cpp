// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "ProjectFile.hpp"
#include "../MCF/Core/Utf8TextFile.hpp"
using namespace MCFBuild;

// 构造函数和析构函数。
ProjectFile::ProjectFile(const MCF::WideString &wcsFullPath){
	MCF::Utf8TextFileReader vReader(MCF::File::Open(wcsFullPath, MCF::File::TO_READ));
	MCF::Utf8String u8sContents;

	vReader.ReadTillEof(u8sContents);
	MCF::WideString wcsContents(u8sContents);

	const auto vResult = xm_vData.Parse(wcsContents);
	if(vResult.first != MCF::Notation::ERR_NONE){
		static const MCF::WideStringObserver ERR_DESC[] = {
			L"ERR_UNKNOWN"_wso,
			L"ERR_NO_VALUE_NAME"_wso,
			L"ERR_NO_PACKAGE_NAME"_wso,
			L"ERR_UNEXCEPTED_PACKAGE_CLOSE"_wso,
			L"ERR_EQU_EXPECTED"_wso,
			L"ERR_UNCLOSED_PACKAGE"_wso,
			L"ERR_SOURCE_PACKAGE_NOT_FOUND"_wso,
			L"ERR_DUPLICATE_PACKAGE"_wso,
			L"ERR_DUPLICATE_VALUE"_wso,
		};

		FORMAT_THROW(
			ERROR_INVALID_DATA,
			L"PROJ_FILE_INVALID\0"_ws
			+ (((std::size_t)vResult.first < COUNT_OF(ERR_DESC)) ? ERR_DESC[vResult.first] : ERR_DESC[0])
		);
	}
}

// 其他非静态成员函数。
