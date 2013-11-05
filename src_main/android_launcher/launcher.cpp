//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: Defines the entry point for the application.
//
//===========================================================================//

#include <errno.h>
#include <cstd/stdio.h>
#include <stdlib.h>
#include <cstd/string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "tier0/icommandline.h"
#include "engine_launcher_api.h"
#include "tier1/interface.h"
#include "tier0/dbg.h"
#include "appframework/iappsystem.h"
#include "appframework/appframework.h"
#include "tier0/platform.h"
#include "tier0/memalloc.h"
#include "filesystem.h"
#include "materialsystem/imaterialsystem.h"
#include "vphysics_interface.h"
#include "filesystem_init.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "tier3/tier3.h"
#include "android_native_app_glue.h"
#include "android_system.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DEFAULT_HL2_GAMEDIR "hl2"

static IEngineAPI *g_pEngineAPI;

static char g_szBasedir[MAX_PATH];

SpewRetval_t LauncherDefaultSpewFunc(SpewType_t spewType, const char *pMsg)
{
	Plat_DebugString(pMsg);
	switch (spewType)
	{
	case SPEW_MESSAGE:
	case SPEW_WARNING:
	case SPEW_LOG:
		return SPEW_CONTINUE;
	case SPEW_ERROR:
		// D0GTODO: Make the engine not restarted in case of a crash
		exit(1);
		return SPEW_DEBUGGER;
	case SPEW_ASSERT:
	default:
		return SPEW_DEBUGGER;
	}
}

class CSourceAppSystemGroup : public CSteamAppSystemGroup
{
public:
	virtual bool Create();
	virtual bool PreInit();
	virtual int Main();
	virtual void PostShutdown();
	virtual void Destroy();
};

//-----------------------------------------------------------------------------
// Instantiate all main libraries
//-----------------------------------------------------------------------------
bool CSourceAppSystemGroup::Create()
{
	AppSystemInfo_t appSystems[] =
	{
		{"vphysics",	VPHYSICS_INTERFACE_VERSION},
		{"", ""}		// Required to terminate the list
	};
	if (!(AddSystems(appSystems))) 
		return false;
	return true;
}

bool CSourceAppSystemGroup::PreInit()
{
	CreateInterfaceFn factory = GetFactory();
	ConnectTier1Libraries(&factory, 1);
	ConVar_Register();
	ConnectTier2Libraries(&factory, 1);
	ConnectTier3Libraries(&factory, 1);
	// D0GTODO: check for the material system too
	if (!g_pFullFileSystem)
		return false;

	CFSSteamSetupInfo steamInfo;
	steamInfo.m_bToolsMode = false;
	steamInfo.m_bSetSteamDLLPath = false;
	steamInfo.m_bSteam = false;
	steamInfo.m_bOnlyUseDirectoryName = true;
	steamInfo.m_pDirectoryName = CommandLine()->ParmValue("-game", DEFAULT_HL2_GAMEDIR);
	if (FileSystem_SetupSteamEnvironment(steamInfo) != FS_OK)
		return false;

	CFSMountContentInfo fsInfo;
	fsInfo.m_pFileSystem = g_pFullFileSystem;
	fsInfo.m_bToolsMode = false;
	fsInfo.m_pDirectoryName = steamInfo.m_GameInfoPath;
	if (FileSystem_MountContent(fsInfo) != FS_OK)
		return false;

	fsInfo.m_pFileSystem->AddSearchPath("platform", steamInfo.m_GameInfoPath);

	StartupInfo_t info;
	info.m_pInstance = GetAppInstance();
	info.m_pBaseDirectory = g_szBasedir;
	info.m_pInitialMod = steamInfo.m_pDirectoryName;
	info.m_pInitialGame = steamInfo.m_pDirectoryName;
	info.m_pParentAppSystemGroup = this;
	info.m_bTextMode = false;
	// D0GTODO: g_pEngineAPI->SetStartupInfo(info);

	return true;
}

int CSourceAppSystemGroup::Main()
{
	for (;;);
	// D0GTODO: g_pEngineAPI->Run()
}

void CSourceAppSystemGroup::PostShutdown()
{
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	ConVar_Unregister();
	DisconnectTier1Libraries();
}

void CSourceAppSystemGroup::Destroy() 
{
	g_pEngineAPI = NULL;
	g_pMaterialSystem = NULL;
}

// Remove all but the last -game parameter.
// This is for mods based off something other than Half-Life 2 (like HL2MP mods).
// The Steam UI does 'steam -applaunch 320 -game c:\steam\steamapps\sourcemods\modname', but applaunch inserts
// its own -game parameter, which would supercede the one we really want if we didn't intercede here.
void RemoveSpuriousGameParameters()
{
	// Find the last -game parameter.
	int nGameArgs = 0;
	char lastGameArg[MAX_PATH];
	for (int i = 0; i < CommandLine()->ParmCount() - 1; ++i)
	{
		if (!Q_stricmp(CommandLine()->GetParm(i), "-game"))
		{
			Q_snprintf(lastGameArg, sizeof(lastGameArg), "\"%s\"", CommandLine()->GetParm(i + 1));
			++nGameArgs;
			++i;
		}
	}

	// We only care if > 1 was specified.
	if (nGameArgs > 1)
	{
		CommandLine()->RemoveParm("-game");
		CommandLine()->AppendParm("-game", lastGameArg);
	}
}

extern "C" void LauncherMain(struct android_app *app)
{
	ANDR_InitActivity(app->activity);
	SetAppInstance(app);
	SpewOutputFunc(LauncherDefaultSpewFunc);

	const char *packageName = ANDR_GetPackageName();

	strcpy(g_szBasedir, "/mnt/sdcard/Android/");
	mkdir(g_szBasedir, 0777);
	strcat(g_szBasedir, "data/");
	mkdir(g_szBasedir, 0777);
	strcat(g_szBasedir, packageName);
	strcat(g_szBasedir, "/");
	mkdir(g_szBasedir, 0777);
	strcat(g_szBasedir, "files/");
	mkdir(g_szBasedir, 0777);
	if (chdir(g_szBasedir))
		Error("LauncherMain: Couldn't change current directory");

	// D0GTODO: copy binaries and gameinfo from the assets

	{
		char commandLine[512];
		strcpy(commandLine, packageName);
		size_t length = strlen(commandLine);
		FILE *f = fopen("commandline.txt", "rb");
		if (f)
		{
			commandLine[length++] = ' ';
			length += fread(commandLine + length, 1, (sizeof(commandLine) - 1) - length, f);
			fclose(f);
			commandLine[length] = '\0';
		}
		CommandLine()->CreateCmdLine(commandLine);
	}
	RemoveSpuriousGameParameters();
	CommandLine()->AppendParm("-insecure", NULL);

	bool restart = true;
	while (restart)
	{
		restart = false;
		CSourceAppSystemGroup sourceSystems;
		CSteamApplication steamApplication(&sourceSystems);
		int retval = steamApplication.Run();
		if (steamApplication.GetErrorStage() == CSourceAppSystemGroup::INITIALIZATION)
			restart = (retval == INIT_RESTART);
		else if (retval == RUN_RESTART)
			restart = true;
	}
}