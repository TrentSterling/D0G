//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Android definitions
//
//===========================================================================//

#if !defined(SRC_ANDROID_SYSTEM_H) && defined(__ANDROID__)
#define SRC_ANDROID_SYSTEM_H

#include "android_native_app_glue.h"
#include <jni.h>
#include "tier0/platform.h"

typedef void (*ANDR_APPCMDHANDLER)(struct android_app *app, int32_t cmd);
typedef int32_t (*ANDR_INPUTHANDLER)(struct android_app *app, AInputEvent *aEvent);

PLATFORM_INTERFACE struct android_app *ANDR_GetApp(void);
PLATFORM_INTERFACE const char *ANDR_GetLanguageString(void);
PLATFORM_INTERFACE const char *ANDR_GetLibraryPath(void);
PLATFORM_INTERFACE const char *ANDR_GetPackageName(void);
PLATFORM_INTERFACE void ANDR_InitApp(struct android_app *app);

// Must not be nested!
PLATFORM_INTERFACE JNIEnv *ANDR_JNIBegin(void);
PLATFORM_INTERFACE void ANDR_JNIEnd(void);

#endif // !SRC_ANDROID_SYSTEM_H && __ANDROID__