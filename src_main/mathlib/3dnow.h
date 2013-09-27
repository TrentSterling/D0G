//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: 
//
//===========================================================================//

#if !defined(_3DNOW_H) && !defined(_LINUX)
#define _3DNOW_H

float _3DNow_Sqrt(float x);
float _3DNow_RSqrt(float x);
float FASTCALL _3DNow_VectorNormalize (Vector& vec);
void FASTCALL _3DNow_VectorNormalizeFast (Vector& vec);
float _3DNow_InvRSquared(const float* v);

#endif // !_3DNOW_H && !_LINUX