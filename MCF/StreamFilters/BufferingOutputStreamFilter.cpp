// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "BufferingOutputStreamFilter.hpp"

namespace MCF {

BufferingOutputStreamFilter::~BufferingOutputStreamFilter(){
}

void BufferingOutputStreamFilter::Put(unsigned char byData){
	y_vStream.Put(byData);
	y_vStream.FlushBuffer(y_vStream.kFlushStreamAuto);
}

void BufferingOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	y_vStream.Put(pData, uSize);
	y_vStream.FlushBuffer(y_vStream.kFlushStreamAuto);
}

void BufferingOutputStreamFilter::Flush(bool bHard){
	y_vStream.Flush(bHard);
}

}
