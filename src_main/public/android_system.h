//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: Android definitions
//
//===========================================================================//

#if !defined(SRC_ANDROID_SYSTEM_H) && defined(__ANDROID__)
#define SRC_ANDROID_SYSTEM_H

#include <android/native_activity.h>
#include <jni.h>

PLATFORM_INTERFACE ANativeActivity *ANDR_GetActivity(void);

PLATFORM_INTERFACE const char *ANDR_GetLanguageString(void);

PLATFORM_INTERFACE const char *ANDR_GetPackageName(void);

PLATFORM_INTERFACE void ANDR_InitActivity(ANativeActivity *activity);

// Must not be nested!
PLATFORM_INTERFACE JNIEnv *ANDR_JNIBegin(void);
PLATFORM_INTERFACE void ANDR_JNIEnd(void);

#endif // !SRC_ANDROID_SYSTEM_H && __ANDROID__