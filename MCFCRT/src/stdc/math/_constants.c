// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "_constants.h"

alignas(16) const double           __MCFCRT_kMath_Pos_1_0      = + 1.0;
alignas(16) const double           __MCFCRT_kMath_Neg_1_0      = - 1.0;

alignas(16) const double           __MCFCRT_kMath_Pos_0_5      = + 0.5;
alignas(16) const double           __MCFCRT_kMath_Neg_0_5      = - 0.5;

alignas(16) const long double      __MCFCRT_kMath_Pos_1p61_Pi  = + 0x1p61 * 3.1415926535897932384626433832795029l;

alignas(16) const uint16_t         __MCFCRT_kI16Max_p8[8]      = { 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, };
alignas(16) const uint32_t         __MCFCRT_kI32Max_p4[4]      = { 0x7FFFFFFF,     0x7FFFFFFF,     0x7FFFFFFF,     0x7FFFFFFF,     };
alignas(16) const uint64_t         __MCFCRT_kI64Max_p2[2]      = { 0x7FFFFFFFFFFFFFFF,             0x7FFFFFFFFFFFFFFF,             };
