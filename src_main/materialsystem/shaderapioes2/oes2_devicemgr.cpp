//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 device manager API.
//
//===========================================================================//
#include "filesystem.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "oes2.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

class CShaderAPIConVarAccessor : public IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(ConCommandBase *pCommand)
	{
		// Link to engine's list instead
		g_pCVar->RegisterConCommand(pCommand);
		const char *pValue = g_pCVar->GetCommandLineValue(pCommand->GetName());
		if (pValue && !pCommand->IsCommand())
		{
			((ConVar *)pCommand)->SetValue(pValue);
		}
		return true;
	}
};

static CreateInterfaceFn s_TempFactory;
void *ShaderDeviceFactory(const char *pName, int *pReturnCode)
{
	if (pReturnCode)
		*pReturnCode = IFACE_OK;
	void *pInterface = s_TempFactory(pName, pReturnCode);
	if (pInterface)
		return pInterface;
	pInterface = Sys_GetFactoryThis()(pName, pReturnCode);
	if (pInterface)
		return pInterface;
	if (pReturnCode)
		*pReturnCode = IFACE_FAILED;
	return NULL;
}

bool CShaderDeviceMgrOES2::Connect(CreateInterfaceFn factory)
{
	s_TempFactory = factory;
	CreateInterfaceFn actualFactory = ShaderDeviceFactory;
	ConnectTier1Libraries(&actualFactory, 1);
	static CShaderAPIConVarAccessor g_ConVarAccessor;
	if (g_pCVar)
		ConVar_Register(0, &g_ConVarAccessor);
	ConnectTier2Libraries(&actualFactory, 1);
	g_pShaderUtil = (IShaderUtil *)(ShaderDeviceFactory(SHADER_UTIL_INTERFACE_VERSION, NULL));
	s_TempFactory = NULL;
	if (!g_pShaderUtil || !g_pFullFileSystem)
	{
		Warning("ShaderAPIOES2 was unable to access the required interfaces!\n");
		return false;
	}
	MathLib_Init(2.2f, 2.2f, 0.0f, 2.0f);
	return true;
}

void CShaderDeviceMgrOES2::Disconnect(void)
{
	g_pShaderUtil = NULL;
	DisconnectTier2Libraries();
	ConVar_Unregister();
	DisconnectTier1Libraries();
}

void CShaderDeviceMgrOES2::GetModeInfo(ShaderDisplayMode_t *pInfo, int nAdapter, int nMode) const
{
	Assert(pInfo->m_nVersion == SHADER_DISPLAY_MODE_VERSION);
	Assert(!nMode);
	ShaderAPI()->GetWindowSize(pInfo->m_nWidth, pInfo->m_nHeight);
	pInfo->m_Format = IMAGE_FORMAT_RGBA8888;
	pInfo->m_nRefreshRateNumerator = 60;
	pInfo->m_nRefreshRateDenominator = 1;
}

InitReturnVal_t CShaderDeviceMgrOES2::Init(void)
{
	HardwareConfig()->DetermineHardwareCapsFromPbuffer();
	return INIT_OK;
}

void *CShaderDeviceMgrOES2::QueryInterface(const char *pInterfaceName)
{
	if (!Q_stricmp(pInterfaceName, SHADER_DEVICE_MGR_INTERFACE_VERSION))
		return static_cast<IShaderDeviceMgr *>(this);
	if (!Q_stricmp(pInterfaceName, MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION))
		return static_cast<IMaterialSystemHardwareConfig *>(HardwareConfig());
	return NULL;
}

void *CShaderDeviceMgrOES2::ShaderInterfaceFactory(const char *pInterfaceName, int *pReturnCode)
{
	if (pReturnCode)
		*pReturnCode = IFACE_OK;

	if (!Q_stricmp(pInterfaceName, SHADER_DEVICE_INTERFACE_VERSION))
		return static_cast<IShaderDevice *>(ShaderAPI());
	if (!Q_stricmp(pInterfaceName, SHADERAPI_INTERFACE_VERSION))
		return static_cast<IShaderAPI *>(ShaderAPI());
	if (!Q_stricmp(pInterfaceName, SHADERSHADOW_INTERFACE_VERSION))
		return static_cast<IShaderShadow *>(ShaderShadow());

	if (pReturnCode)
		*pReturnCode = IFACE_FAILED;
	return NULL;
}

void CShaderDeviceMgrOES2::Shutdown(void)
{
	ShaderDeviceInfo_t info;
	ShaderAPI()->SetMode(NULL, 0, info);
}

static CShaderDeviceMgrOES2 s_ShaderDeviceMgrOES2;
CShaderDeviceMgrOES2 *g_pShaderDeviceMgrOES2 = &s_ShaderDeviceMgrOES2;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CShaderDeviceMgrOES2, IShaderDeviceMgr,
	SHADER_DEVICE_MGR_INTERFACE_VERSION, s_ShaderDeviceMgrOES2);