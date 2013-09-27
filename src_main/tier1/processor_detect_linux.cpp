//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: linux dependant ASM code for CPU capability detection
//
// $Workfile:     $
// $NoKeywords: $
//==========================================================================//

#ifndef __arm__
#include <cpuid.h>
#define cpuid(in,a,b,c,d) __get_cpuid(in, &(a), &(b), &(c), &(d))
#include "tier0/platform.h"
#endif

bool CheckMMXTechnology(void)
{
#ifdef __arm__
	return false;
#else
    uint32 eax,ebx,edx=0,unused;
    cpuid(1,eax,ebx,unused,edx);

    return edx & 0x800000;
#endif
}

bool CheckSSETechnology(void)
{
#ifdef __arm__
#ifdef __ARM_NEON__
	return true;
#else
	return false;
#endif
#else
    uint32 eax,ebx,edx=0,unused;
    cpuid(1,eax,ebx,unused,edx);

    return edx & 0x2000000L;
#endif
}

bool CheckSSE2Technology(void)
{
#ifdef __arm__
	return false;
#else
    uint32 eax,ebx,edx=0,unused;
    cpuid(1,eax,ebx,unused,edx);

    return edx & 0x04000000;
#endif
}

bool Check3DNowTechnology(void)
{
#ifdef __arm__
	return false;
#else
    uint32 eax = 0, unused;
    cpuid(0x80000000,eax,unused,unused,unused);

    if ( eax > 0x80000000L )
    {
     	cpuid(0x80000001,unused,unused,unused,eax);
		return ( eax & 1<<31 );
    }
    return false;
#endif
}
