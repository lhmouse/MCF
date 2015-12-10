// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Utf8TextFile.hpp"
#include "../Utilities/BinaryOperations.hpp"

namespace MCF {

namespace {
	constexpr unsigned char kUtf8Bom[] = { 0xEF, 0xBB, 0xBF };
}

// ========== Utf8TextFileReader ==========
// 构造函数和析构函数。
Utf8TextFileReader::Utf8TextFileReader(File &&vFile)
	: x_vFile(std::move(vFile))
	, x_u64Offset(0), x_sbufCache()
{
	if(x_vFile){
		unsigned char abyTemp[sizeof(kUtf8Bom)];
		if((x_vFile.Read(abyTemp, sizeof(abyTemp), 0) == sizeof(abyTemp)) && (BComp(abyTemp, kUtf8Bom) == 0)){
			x_u64Offset += sizeof(kUtf8Bom);
		}
	}
}
Utf8TextFileReader::~Utf8TextFileReader(){
}

// 其他非静态成员函数。
void Utf8TextFileReader::Reset() noexcept {
	Utf8TextFileReader().Swap(*this);
}
void Utf8TextFileReader::Reset(File &&vFile){
	Utf8TextFileReader(std::move(vFile)).Swap(*this);
}

bool Utf8TextFileReader::IsAtEndOfFile() const {
	return x_sbufCache.IsEmpty() && (x_u64Offset == x_vFile.GetSize());
}
int Utf8TextFileReader::Read(){
	if(x_sbufCache.GetSize() <= 1){
		unsigned char abyTemp[1024];
		const auto uBytesRead = x_vFile.Read(abyTemp, sizeof(abyTemp), x_u64Offset);
		x_sbufCache.Put(abyTemp, uBytesRead);
		x_u64Offset += uBytesRead;
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
		u8sData.UncheckedPush(static_cast<char>(nChar));
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
		u8sData.Push(static_cast<char>(nChar));
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
		u8sData.Push(static_cast<char>(nChar));
		if((nChar = Read()) < 0){
			break;
		}
	}
	return true;
}

// ========== Utf8TextFileWriter ==========
// 构造函数和析构函数。
Utf8TextFileWriter::Utf8TextFileWriter(File &&vFile, std::uint32_t u32Flags)
	: x_vFile(std::move(vFile)), x_u32Flags(u32Flags)
	, x_u64Offset(x_vFile.GetSize()), x_u8sLine()
{
	if(x_vFile && (x_u32Flags & kHasBom) && (x_u64Offset == 0)){
		x_vFile.Write(0, kUtf8Bom, sizeof(kUtf8Bom));
		x_vFile.Flush();

		x_u64Offset += sizeof(kUtf8Bom);
	}
}
Utf8TextFileWriter::~Utf8TextFileWriter(){
	try {
		Flush();
	} catch(...){
	}
}

// 其他非静态成员函数。
void Utf8TextFileWriter::Reset() noexcept {
	Utf8TextFileWriter().Swap(*this);
}
void Utf8TextFileWriter::Reset(File &&vFile, std::uint32_t u32Flags){
	Utf8TextFileWriter(std::move(vFile), u32Flags).Swap(*this);
}

void Utf8TextFileWriter::Write(char ch){
	if(x_u32Flags & kUnbuffered){
		x_vFile.Write(x_u64Offset, &ch, 1);
	} else {
		if((ch == '\n') && (x_u32Flags & kEndlCrLf)){
			x_u8sLine += '\r';
		}
		x_u8sLine += ch;

		bool bFlushNow;
		if(x_u32Flags & kFullBuffered){
			bFlushNow = (x_u8sLine.GetSize() >= 0x1000);
		} else {
			bFlushNow = (ch == '\n');
		}
		if(bFlushNow){
			Flush();
		}
	}
}
void Utf8TextFileWriter::Write(const Utf8StringView &u8svData){
	for(auto p = u8svData.GetBegin(); p != u8svData.GetEnd(); ++p){
		Write(*p);
	}
}
void Utf8TextFileWriter::WriteLine(const Utf8StringView &u8svData){
	Write(u8svData);
	Write('\n');
}
void Utf8TextFileWriter::Flush(){
	x_vFile.Write(x_u64Offset, x_u8sLine.GetData(), (std::uint32_t)x_u8sLine.GetSize());
	x_u64Offset += x_u8sLine.GetSize();
	x_u8sLine.Clear();

	x_vFile.Flush();
}

}
