// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "ArchiveClass.hpp"
using namespace MCF;

// MCF 标准字符串类序列化/反序列化器的实例化静态成员函数定义。
void xArchiveBaseClass::xArchiver<MBString>::Serialize(OutArchiveClass &dst, const MBString &src) throw() {
	DWORD dwStrLen = (DWORD)src.GetLength();
	dst <<dwStrLen;
	dst.InsertArray(src.GetBuffer(), dwStrLen);
}
void xArchiveBaseClass::xArchiver<MBString>::Unserialize(MBString &dst, InArchiveClass &src){
	DWORD dwStrLen;
	src >>dwStrLen;
	dst.Resize((std::size_t)dwStrLen, false);
	src.ExtractArray(dst.GetBuffer(), dwStrLen);
}

void xArchiveBaseClass::xArchiver<WCString>::Serialize(OutArchiveClass &dst, const WCString &src) throw() {
	DWORD dwStrLen = (DWORD)src.GetLength();
	dst <<dwStrLen;
	dst.InsertArray(src.GetBuffer(), dwStrLen);
}
void xArchiveBaseClass::xArchiver<WCString>::Unserialize(WCString &dst, InArchiveClass &src){
	DWORD dwStrLen;
	src >>dwStrLen;
	dst.Resize((std::size_t)dwStrLen, false);
	src.ExtractArray(dst.GetBuffer(), dwStrLen);
}

void xArchiveBaseClass::xArchiver<UTF8String>::Serialize(OutArchiveClass &dst, const UTF8String &src) throw() {
	DWORD dwStrLen = (DWORD)src.GetLength();
	dst <<dwStrLen;
	dst.InsertArray(src.GetBuffer(), dwStrLen);
}
void xArchiveBaseClass::xArchiver<UTF8String>::Unserialize(UTF8String &dst, InArchiveClass &src){
	DWORD dwStrLen;
	src >>dwStrLen;
	dst.Resize((std::size_t)dwStrLen, false);
	src.ExtractArray(dst.GetBuffer(), dwStrLen);
}
