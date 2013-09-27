//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: Android win32 replacements - Mocks trivial windows flow
//
//===========================================================================//

#ifndef ANDROID_WIN32STUBS_H
#define ANDROID_WIN32STUBS_H

#define THREAD_PRIORITY_IDLE 0
#define THREAD_PRIORITY_LOWEST 1
#define THREAD_PRIORITY_BELOW_NORMAL 25
#define THREAD_PRIORITY_NORMAL 50
#define THREAD_PRIORITY_ABOVE_NORMAL 75
#define THREAD_PRIORITY_HIGHEST 99

typedef struct waveformatex_tag {
	unsigned short wFormatTag;
	unsigned short nChannels;
	unsigned int nSamplesPerSec;
	unsigned int nAvgBytesPerSec;
	unsigned short nBlockAlign;
	unsigned short wBitsPerSample;
	unsigned short cbSize;
} WAVEFORMATEX;

typedef struct adpcmcoef_tag {
	short iCoef1;
	short iCoef2;
} ADPCMCOEFSET;

typedef struct adpcmwaveformat_tag {
	WAVEFORMATEX wfx;
	unsigned short wSamplesPerBlock;
	unsigned short wNumCoef;
	ADPCMCOEFSET aCoef[1];
} ADPCMWAVEFORMAT;

#endif // ANDROID_WIN32STUBS_H