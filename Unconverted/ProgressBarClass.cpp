// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "ProgressBarClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
ProgressBarClass::ProgressBarClass(){
}
ProgressBarClass::~ProgressBarClass(){
}

// 其他非静态成员函数。
void ProgressBarClass::xPostSubclass(){
	WindowBaseClass::xPostSubclass();

	SendMessage(PBM_SETRANGE32, 0, 0xFFFF);
	SetProgress(0);
}

bool ProgressBarClass::IsSmooth() const {
	return (GetStyle() & PBS_SMOOTH) != 0;
}
void ProgressBarClass::SetSmooth(bool bToSet){
	if(bToSet && !IsSmooth()){
		SetStyle(PBS_SMOOTH, 0xFFFFFFFF);
	} else if(!bToSet && !IsSmooth()){
		SetStyle(PBS_SMOOTH, 0);
	}
}

WORD ProgressBarClass::GetProgress() const {
	return (WORD)SendMessage(PBM_GETPOS, 0, 0);
}
void ProgressBarClass::SetProgress(WORD wNewProgress){
	SendMessage(PBM_SETPOS, (WPARAM)wNewProgress, 0);
}

bool ProgressBarClass::IsMarquee() const {
	return (GetStyle() & PBS_MARQUEE) != 0;
}
void ProgressBarClass::SetMarquee(bool bToSet, unsigned int uInterval){
	if(bToSet && !IsMarquee()){
		SetStyle(PBS_MARQUEE, 0xFFFFFFFF);
		SendMessage(PBM_SETMARQUEE, TRUE, (LPARAM)uInterval);
		Invalidate();
	} else if(!bToSet && IsMarquee()){
		SendMessage(PBM_SETMARQUEE, FALSE, (LPARAM)uInterval);
		SetStyle(PBS_MARQUEE, 0);
		SetProgress(0);
		Invalidate();
	}
}
