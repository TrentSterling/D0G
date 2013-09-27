//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: 
//
//===========================================================================//

#if !defined(_SSE_H) && !defined(__arm__)
#define _SSE_H

float _SSE_Sqrt(float x);
float _SSE_RSqrtAccurate(float a);
float _SSE_RSqrtFast(float x);
float FASTCALL _SSE_VectorNormalize(Vector& vec);
void FASTCALL _SSE_VectorNormalizeFast(Vector& vec);
float _SSE_InvRSquared(const float* v);
#if defined(_WIN32)
void _SSE_SinCos(float x, float* s, float* c);
float _SSE_cos( float x);
void _SSE2_SinCos(float x, float* s, float* c);
float _SSE2_cos(float x);
#endif

#endif // !_SSE_H && !__arm__