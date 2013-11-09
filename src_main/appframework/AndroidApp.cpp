//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: An application framework.
//
//===========================================================================//

#include "appframework/AppFramework.h"
#include "tier0/dbg.h"
#include "interface.h"
#include "filesystem.h"
#include "appframework/iappsystemgroup.h"
#include "filesystem_init.h"
#include "vstdlib/cvar.h"
#include "android_system.h"

SpewRetval_t AndroidAppDefaultSpewFunc(SpewType_t spewType, const char *pMsg)
{
	Plat_DebugString(pMsg);
	switch (spewType)
	{
	case SPEW_MESSAGE:
	case SPEW_WARNING:
	case SPEW_LOG:
		return SPEW_CONTINUE;
	case SPEW_ASSERT:
	case SPEW_ERROR:
	default:
		return SPEW_DEBUGGER;
	}
}

SpewOutputFunc_t g_DefaultSpewFunc = AndroidAppDefaultSpewFunc;

void *GetAppInstance()
{
	return ANDR_GetApp();
}

void SetAppInstance(void *hInstance)
{
}

//-----------------------------------------------------------------------------
//
// Default implementation of an application meant to be run on Android
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CSteamApplication::CSteamApplication(CSteamAppSystemGroup *pAppSystemGroup)
{
	m_pChildAppSystemGroup = pAppSystemGroup;
	m_pFileSystem = NULL;
	m_bSteam = false;
}

//-----------------------------------------------------------------------------
// Create necessary interfaces
//-----------------------------------------------------------------------------
bool CSteamApplication::Create()
{
	FileSystem_SetErrorMode( FS_ERRORMODE_AUTO );

	char pFileSystemDLL[MAX_PATH];
	if ( FileSystem_GetFileSystemDLLName( pFileSystemDLL, MAX_PATH, m_bSteam ) != FS_OK )
		return false;
	
	FileSystem_SetupSteamInstallPath();

	// Add in the cvar factory
	AppModule_t cvarModule = LoadModule( VStdLib_GetICVarFactory() );
	AddSystem( cvarModule, CVAR_INTERFACE_VERSION );

	AppModule_t fileSystemModule = LoadModule( pFileSystemDLL );
	m_pFileSystem = (IFileSystem*)AddSystem( fileSystemModule, FILESYSTEM_INTERFACE_VERSION );
	if ( !m_pFileSystem )
	{
		Error( "Unable to load %s", pFileSystemDLL );
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// The file system pointer is invalid at this point
//-----------------------------------------------------------------------------
void CSteamApplication::Destroy()
{
	m_pFileSystem = NULL;
}

//-----------------------------------------------------------------------------
// Pre-init, shutdown
//-----------------------------------------------------------------------------
bool CSteamApplication::PreInit()
{
	return true;
}

void CSteamApplication::PostShutdown()
{
}

//-----------------------------------------------------------------------------
// Run steam main loop
//-----------------------------------------------------------------------------
int CSteamApplication::Main()
{
	// Now that Steam is loaded, we can load up main libraries through steam
	if ( FileSystem_SetBasePaths( m_pFileSystem ) != FS_OK )
		return 0;

	m_pChildAppSystemGroup->Setup( m_pFileSystem, this );
	return m_pChildAppSystemGroup->Run();
}

//-----------------------------------------------------------------------------
// Use this version in cases where you can't control the main loop and
// expect to be ticked
//-----------------------------------------------------------------------------
int CSteamApplication::Startup()
{
	int nRetVal = BaseClass::Startup();
	if ( GetErrorStage() != NONE )
		return nRetVal;

	if ( FileSystem_SetBasePaths( m_pFileSystem ) != FS_OK )
		return 0;

	// Now that Steam is loaded, we can load up main libraries through steam
	m_pChildAppSystemGroup->Setup( m_pFileSystem, this );
	return m_pChildAppSystemGroup->Startup();
}

void CSteamApplication::Shutdown()
{
	m_pChildAppSystemGroup->Shutdown();
	BaseClass::Shutdown();
}