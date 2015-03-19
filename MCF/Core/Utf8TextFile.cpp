// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Utf8TextFile.hpp"
#include "../Utilities/BinaryOperations.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

namespace {
	constexpr unsigned char UTF8_BOM[] = { 0xEF, 0xBB, 0xBF };
}

// ========== Utf8TextFileReader ==========
// 其他非静态成员函数。
void Utf8TextFileReader::Reset() noexcept {
	x_vFile.Close();
	x_u64Offset = 0;
	x_sbufCache.Clear();
}
void Utf8TextFileReader::Reset(File &&vFile){
	ASSERT(&x_vFile != &vFile);

	Reset();

	if(!vFile){
		return;
	}
	x_vFile = std::move(vFile);

	unsigned char abyTemp[sizeof(UTF8_BOM)];
	if((x_vFile.Read(abyTemp, sizeof(abyTemp), 0) == sizeof(abyTemp)) && !BComp(abyTemp, UTF8_BOM)){
		x_u64Offset += sizeof(UTF8_BOM);
	}
}

bool Utf8TextFileReader::IsAtEndOfFile() const {
	return x_sbufCache.IsEmpty() && (x_u64Offset == x_vFile.GetSize());
}
int Utf8TextFileReader::Read(){
	if(x_sbufCache.GetSize() <= 1){
		unsigned char abyTemp[0x1000];
		const auto uBytesRead = x_vFile.Read(abyTemp, sizeof(abyTemp), x_u64Offset);
		if(uBytesRead != 0){
			x_sbufCache.Put(abyTemp, uBytesRead);
			x_u64Offset += uBytesRead;
		}
	}
	int nRet = x_sbufCache.Get();
	if((nRet == '\r') && (x_sbufCache.Peek() == '\n')){
		nRet = x_sbufCache.Get();
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
		u8sData.UncheckedPush((char)nChar);
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
// 其他非静态成员函数。
void Utf8TextFileWriter::Reset() noexcept {
	try {
		Flush();
	} catch(...){
	}
	x_vFile.Close();
	x_u32Flags = 0;
	x_u64Offset = 0;
	x_u8sLine.Clear();
}
void Utf8TextFileWriter::Reset(File &&vFile, std::uint32_t u32Flags){
	ASSERT(&x_vFile != &vFile);

	x_vFile = std::move(vFile);
	x_u32Flags = u32Flags;
	x_u64Offset = x_vFile.GetSize();

	if((x_u32Flags & BOM_USE) && (x_u64Offset == 0)){
		x_vFile.Write(0, UTF8_BOM, sizeof(UTF8_BOM));
		x_vFile.Flush();

		x_u64Offset += sizeof(UTF8_BOM);
	}

}

void Utf8TextFileWriter::Write(char ch){
	if(x_u32Flags & BUF_NONE){
		x_vFile.Write(x_u64Offset, &ch, 1);
	} else {
		if((ch == '\n') && (x_u32Flags & LES_CRLF)){
			x_u8sLine += '\r';
		}
		x_u8sLine += ch;

		bool bFlushNow;
		if(x_u32Flags & BUF_FULL){
			bFlushNow = x_u8sLine.GetSize() >= 0x1000;
		} else {
			bFlushNow = (ch == '\n');
		}
		if(bFlushNow){
			Flush();
		}
	}
}
void Utf8TextFileWriter::Write(const Utf8StringObserver &u8soData){
	for(auto p = u8soData.GetBegin(); p != u8soData.GetEnd(); ++p){
		Write(*p);
	}
}
void Utf8TextFileWriter::WriteLine(const Utf8StringObserver &u8soData){
	Write(u8soData);
	Write('\n');
}
void Utf8TextFileWriter::Flush(){
	x_vFile.Write(x_u64Offset, x_u8sLine.GetData(), x_u8sLine.GetSize());
	x_u8sLine.Clear();
	x_u64Offset += x_u8sLine.GetSize();

	x_vFile.Flush();
}

}
