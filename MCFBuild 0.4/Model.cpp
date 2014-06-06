// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Model.hpp"
#include "../MCF/Core/VVector.hpp"
#include "../MCF/Core/System.hpp"
#include "../MCF/Core/Exception.hpp"
#include "../MCF/Core/MultiIndexedMap.hpp"
using namespace MCFBuild;

namespace {

struct Helper : public Model {
	Helper() = default;
};

Helper g_vModel;

}

// 静态成员函数。
Model &Model::GetInstance() noexcept {
	return g_vModel;
}

// 构造函数和析构函数。
Model::Model()
	: xm_bVerbose	(false)
	, xm_bRawOutput	(false)
{
	const auto vecArgV = MCF::GetArgV();
	//xm_bRawOutput = false;
}

// 其他非静态成员函数。
unsigned int Model::Run(){
	return 123;
}
