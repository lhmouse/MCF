// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Utf8TextFile.hpp"
#include "../Utilities/BinaryOperations.hpp"
#include "../Containers/VVector.hpp"
using namespace MCF;

namespace {

constexpr unsigned char UTF8_BOM[] = { 0xEF, 0xBB, 0xBF };

}

// ========== Utf8TextFileReader ==========
// 构造函数和析构函数。
Utf8TextFileReader::Utf8TextFileReader(std::unique_ptr<File> pFile)
	: xm_pFile(std::move(pFile)), xm_u64Offset	(0)
{
	unsigned char abyTemp[sizeof(UTF8_BOM)];
	if((xm_pFile->Read(abyTemp, sizeof(abyTemp), 0) == sizeof(abyTemp)) && !BComp(abyTemp, UTF8_BOM)){
		xm_u64Offset += sizeof(UTF8_BOM);
	}
}

// 其他非静态成员函数。
bool Utf8TextFileReader::IsAtEndOfFile() const noexcept {
	return xm_sbufCache.IsEmpty() && (xm_u64Offset == xm_pFile->GetSize());
}
int Utf8TextFileReader::Read(){
	if(xm_sbufCache.GetSize() <= 1){
		unsigned char abyTemp[0x1000];
		const auto uBytesRead = xm_pFile->Read(abyTemp, sizeof(abyTemp), xm_u64Offset);
		if(uBytesRead != 0){
			xm_sbufCache.Put(abyTemp, uBytesRead);
			xm_u64Offset += uBytesRead;
		}
	}
	int nRet = xm_sbufCache.Get();
	if((nRet == '\r') && (xm_sbufCache.Peek() == '\n')){
		nRet = xm_sbufCache.Get();
	}
	return nRet;
}
bool Utf8TextFileReader::Read(Utf8String &u8sData, std::size_t uCount){
	u8sData.Clear();
	u8sData.Reserve(uCount);

	if(uCount == 0){
		return !IsAtEndOfFile();
	}

	int nChar;
	if((nChar = Read()) < 0){
		return false;
	}
	for(;;){
		u8sData.PushNoCheck((char)nChar);
		if(u8sData.GetSize() == uCount){
			break;
		}
		if((nChar = Read()) < 0){
			break;
		}
	}
	return true;
}
bool Utf8TextFileReader::ReadLine(Utf8String &u8sData){
	u8sData.Clear();

	int nChar;
	if((nChar = Read()) < 0){
		return false;
	}
	for(;;){
		if(nChar == '\n'){
			break;
		}
		u8sData.Push((char)nChar);
		if((nChar = Read()) < 0){
			break;
		}
	}
	return true;
}
bool Utf8TextFileReader::ReadTillEof(Utf8String &u8sData){
	u8sData.Clear();

	int nChar;
	if((nChar = Read()) < 0){
		return false;
	}
	for(;;){
		u8sData.Push((char)nChar);
		if((nChar = Read()) < 0){
			break;
		}
	}
	return true;
}

// ========== Utf8TextFileWriter ==========
// 构造函数和析构函数。
Utf8TextFileWriter::Utf8TextFileWriter(std::unique_ptr<File> pFile, std::uint32_t u32Flags)
	: xm_pFile(std::move(pFile)), xm_u32Flags(u32Flags), xm_u64Offset(0)
{
	xm_u64Offset = xm_pFile->GetSize();

	if((xm_u32Flags & BOM_USE) && (xm_u64Offset == 0)){
		xm_pFile->Write(0, UTF8_BOM, sizeof(UTF8_BOM));
		xm_pFile->Flush();

		xm_u64Offset += sizeof(UTF8_BOM);
	}
}
Utf8TextFileWriter::~Utf8TextFileWriter(){
	try {
		Flush();
	} catch(...){
	}
}

// 其他非静态成员函数。
void Utf8TextFileWriter::Write(char ch){
	if(xm_u32Flags & BUF_NONE){
		xm_pFile->Write(xm_u64Offset, &ch, 1);
	} else {
		if((ch == '\n') && (xm_u32Flags & LES_CRLF)){
			xm_u8sLine += '\r';
		}
		xm_u8sLine += ch;

		bool bFlushNow;
		if(xm_u32Flags & BUF_FULL){
			bFlushNow = xm_u8sLine.GetSize() >= 0x1000;
		} else {
			bFlushNow = (ch == '\n');
		}
		if(bFlushNow){
			Flush();
		}
	}
}
void Utf8TextFileWriter::Write(const Utf8StringObserver &u8soData){
	for(auto ch : u8soData){
		Write(ch);
	}
}
void Utf8TextFileWriter::WriteLine(const Utf8StringObserver &u8soData){
	Write(u8soData);
	Write('\n');
}
void Utf8TextFileWriter::Flush(){
	xm_pFile->Write(xm_u64Offset, xm_u8sLine.GetData(), xm_u8sLine.GetSize());
	xm_pFile->Flush();

	xm_u64Offset += xm_u8sLine.GetSize();
	xm_u8sLine.Clear();
}
