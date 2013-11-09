//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: A redirection tool that allows the DLLs to reside elsewhere.
//
//===========================================================================//

#define _ANDROID_NATIVE_APP_GLUE_FUNCTIONS
#include "android_native_app_glue.h"
#include <android/log.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <cstd/stdio.h>
#include <stdlib.h>
#include <cstd/string.h>

void LauncherMainError(const char *text)
{
	__android_log_print(ANDROID_LOG_ERROR, "Launcher", "%s", text);
	// D0GTODO: Make the engine not restarted in case of a crash
	exit(1);
}

#ifdef __arm__
bool LauncherMainIsNEON(void)
{
	FILE *f = fopen("/proc/cpuinfo", "r");
	if (!f)
		return false;
	bool found = false;
	char line[1024], search_str[] = "Features\t:";
	while (fgets(line, sizeof(line), f))
	{
		if (!strncmp(line, search_str, sizeof(search_str) - 1))
		{
			found = true;
			break;
		}
	}
	fclose(f);
	if (!found)
		return false;
	// Change newline to space to support the case when "neon" is the last feature
	char *newline = line;
	while (*newline)
	{
		if ((*newline == '\r') || (*newline == '\n'))
		{
			*newline = ' ';
			*(newline + 1) = '\0';
			break;
		}
		++newline;
	}
	return strstr(line, " neon ") != NULL;
}
#endif

void android_main(struct android_app *app)
{
	app_dummy();

	char libPath[PATH_MAX];
	strcpy(libPath, "/data/data/");
	{
		JNIEnv *env = NULL;
		app->activity->vm->AttachCurrentThread(&env, NULL);
		if (!env)
			LauncherMainError("Failed to attach the primary thread to the Java VM");
		jmethodID getPackageName = env->GetMethodID(env->GetObjectClass(app->activity->clazz),
			"getPackageName", "()Ljava/lang/String;");
		jstring packageNameHandle = (jstring)(env->CallObjectMethod(app->activity->clazz, getPackageName));
		const char *packageName = env->GetStringUTFChars(packageNameHandle, NULL);
		strcat(libPath, packageName);
		env->ReleaseStringUTFChars(packageNameHandle, packageName);
		app->activity->vm->DetachCurrentThread();
	}
	strcat(libPath, "/lib/lib");
	size_t libLength = strlen(libPath);
	const char *libAddition;
#ifdef __arm__
	if (LauncherMainIsNEON())
		libAddition = "_android_neon.so";
	else
		libAddition = "_android_arm.so";
#else
	libAddition = "_android_x86.so";
#endif

	strcpy(libPath + libLength, "gnustl_shared.so");
	if (!dlopen(libPath, RTLD_NOW | RTLD_GLOBAL))
		LauncherMainError("Failed to load dll gnustl_shared");
	strcpy(libPath + libLength, "tier0");
	strcat(libPath, libAddition);
	if (!dlopen(libPath, RTLD_NOW | RTLD_GLOBAL))
		LauncherMainError("Failed to load dll tier0");
	strcpy(libPath + libLength, "vstdlib");
	strcat(libPath, libAddition);
	if (!dlopen(libPath, RTLD_NOW | RTLD_GLOBAL))
		LauncherMainError("Failed to load dll vstdlib");

	strcpy(libPath + libLength, "android_launcher");
	strcat(libPath, libAddition);
	void *launcher = dlopen(libPath, RTLD_NOW);
	if (!launcher)
		LauncherMainError("Failed to load the launcher DLL");
	union {
		void (*f)(struct android_app *app);
		void *p;
	} main;
	main.p = dlsym(launcher, "LauncherMain");
	if (!main.p)
		LauncherMainError("LauncherMain entry point not found");
	main.f(app);
}