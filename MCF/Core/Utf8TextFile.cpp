// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Utf8TextFile.hpp"
#include "../Core/VVector.hpp"
using namespace MCF;

static constexpr unsigned char UTF8_BOM[] = {0xEF, 0xBB, 0xBF};

// ========== Utf8TextFileReader ==========
// 构造函数和析构函数。
Utf8TextFileReader::Utf8TextFileReader(std::unique_ptr<File> pFile)
	: xm_pFile		(std::move(pFile))
	, xm_u64Offset	(0)
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
			xm_sbufCache.Insert(abyTemp, uBytesRead);
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
	auto i = uCount;
	for(;;){
		u8sData.PushNoCheck(nChar & 0xFF);
		if(--i == 0){
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
		u8sData.Push(nChar & 0xFF);
		if((nChar = Read()) < 0){
			break;
		}
	}
	return true;
}
bool Utf8TextFileReader::ReadAll(Utf8String &u8sData){
	u8sData.Clear();

	int nChar;
	if((nChar = Read()) < 0){
		return false;
	}
	for(;;){
		u8sData.Push(nChar & 0xFF);
		if((nChar = Read()) < 0){
			break;
		}
	}
	return true;
}

// ========== Utf8TextFileWriter ==========
// 构造函数和析构函数。
Utf8TextFileWriter::Utf8TextFileWriter(std::unique_ptr<File> pFile, bool bCrlf, bool bHasBom)
	: xm_pFile		(std::move(pFile))
	, xm_bCrlf		(bCrlf)
	, xm_bHasBom	(bHasBom)
	, xm_u64Offset	(0)
{
	if(xm_bHasBom){
		xm_pFile->Write(0, UTF8_BOM, sizeof(UTF8_BOM));
		xm_u64Offset += sizeof(UTF8_BOM);
	}
}
Utf8TextFileWriter::~Utf8TextFileWriter() noexcept {
	try {
		Flush();
	} catch(...){
	}
}

// 其他非静态成员函数。
void Utf8TextFileWriter::Write(char ch){
	if(ch == '\n'){
		xm_sbufCache.Put((unsigned char)'\r');
	}
	xm_sbufCache.Put((unsigned char)ch);
	if(ch == '\n'){
		Flush();
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
	VVector<unsigned char> vecTemp;
	const auto uBytesToFlush = xm_sbufCache.GetSize();
	vecTemp.Resize(uBytesToFlush);
	xm_sbufCache.Extract(vecTemp.GetData(), uBytesToFlush);
	ASSERT(xm_sbufCache.IsEmpty());

	xm_pFile->Write(xm_u64Offset, vecTemp.GetData(), uBytesToFlush);
	xm_u64Offset += uBytesToFlush;

	xm_pFile->Flush();
}
