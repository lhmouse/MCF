#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	Utf8String s1("中文测试字符串\n");
	Utf16String s2(s1);

	DWORD dwCharsWritten;
	::WriteConsoleW(::GetStdHandle(STD_ERROR_HANDLE), s2.GetData(), s2.GetSize(), &dwCharsWritten, nullptr);

	return 0;
}
