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
// 其他非静态成员函数。
void Utf8TextFileReader::Reset() noexcept {
	$vFile.Close();
	$u64Offset = 0;
	$sbufCache.Clear();
}
void Utf8TextFileReader::Reset(File &&vFile){
	ASSERT(&$vFile != &vFile);

	Reset();

	if(!vFile){
		return;
	}
	$vFile = std::move(vFile);

	unsigned char abyTemp[sizeof(kUtf8Bom)];
	if(($vFile.Read(abyTemp, sizeof(abyTemp), 0) == sizeof(abyTemp)) && !BComp(abyTemp, kUtf8Bom)){
		$u64Offset += sizeof(kUtf8Bom);
	}
}

bool Utf8TextFileReader::IsAtEndOfFile() const {
	return $sbufCache.IsEmpty() && ($u64Offset == $vFile.GetSize());
}
int Utf8TextFileReader::Read(){
	if($sbufCache.GetSize() <= 1){
		unsigned char abyTemp[0x1000];
		const auto uBytesRead = $vFile.Read(abyTemp, sizeof(abyTemp), $u64Offset);
		if(uBytesRead != 0){
			$sbufCache.Put(abyTemp, uBytesRead);
			$u64Offset += uBytesRead;
		}
	}
	int nRet = $sbufCache.Get();
	if((nRet == '\r') && ($sbufCache.Peek() == '\n')){
		nRet = $sbufCache.Get();
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
	$vFile.Close();
	$u32Flags = 0;
	$u64Offset = 0;
	$u8sLine.Clear();
}
void Utf8TextFileWriter::Reset(File &&vFile, std::uint32_t u32Flags){
	ASSERT(&$vFile != &vFile);

	$vFile = std::move(vFile);
	$u32Flags = u32Flags;
	$u64Offset = $vFile.GetSize();

	if(($u32Flags & kHasBom) && ($u64Offset == 0)){
		$vFile.Write(0, kUtf8Bom, sizeof(kUtf8Bom));
		$vFile.Flush();

		$u64Offset += sizeof(kUtf8Bom);
	}

}

void Utf8TextFileWriter::Write(char ch){
	if($u32Flags & kUnbuffered){
		$vFile.Write($u64Offset, &ch, 1);
	} else {
		if((ch == '\n') && ($u32Flags & kEndlCrLf)){
			$u8sLine += '\r';
		}
		$u8sLine += ch;

		bool bFlushNow;
		if($u32Flags & kFullBuffered){
			bFlushNow = $u8sLine.GetSize() >= 0x1000;
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
	$vFile.Write($u64Offset, $u8sLine.GetData(), $u8sLine.GetSize());
	$u8sLine.Clear();
	$u64Offset += $u8sLine.GetSize();

	$vFile.Flush();
}

}
