//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 device manager.
//
//===========================================================================//
#ifndef OES2_DEVMGR_H
#define OES2_DEVMGR_H

#include <cstd/string.h>
#include "tier1/utlvector.h"

class CShaderDeviceMgrOES2 : public IShaderDeviceMgr
{
public:
	virtual bool Connect(CreateInterfaceFn factory);
	virtual void Disconnect(void);
	virtual void *QueryInterface(const char *pInterfaceName);
	virtual InitReturnVal_t Init(void);
	virtual void Shutdown(void);

	virtual int GetAdapterCount(void) const { return 1; }
	virtual void GetAdapterInfo(int nAdapter, MaterialAdapterInfo_t &info) const
	{
		Assert(!nAdapter);
		memcpy(&info, static_cast<const MaterialAdapterInfo_t *>(&(HardwareConfig()->Caps())), sizeof(MaterialAdapterInfo_t));
	}
	// No dxsupport.cfg support (yet?)
	virtual bool GetRecommendedConfigurationInfo(int nAdapter, int nDXLevel, KeyValues *pConfiguration) { return true; }
	virtual int GetModeCount(int nAdapter) const { Assert(!nAdapter); return 1; }
	virtual void GetModeInfo(ShaderDisplayMode_t *pInfo, int nAdapter, int nMode) const;
	virtual void GetCurrentModeInfo(ShaderDisplayMode_t *pInfo, int nAdapter) const { GetModeInfo(pInfo, nAdapter, 0); }
	virtual bool SetAdapter(int nAdapter, int nFlags) { return true; }
	virtual CreateInterfaceFn SetMode(void *hWnd, int nAdapter, const ShaderDeviceInfo_t &mode)
		{ ShaderAPI()->SetMode(hWnd, nAdapter, mode); return ShaderInterfaceFactory; }
	virtual void AddModeChangeCallback(ShaderModeChangeCallbackFunc_t func)
		{ Assert(func && (m_ModeChangeCallbacks.Find(func) < 0)); m_ModeChangeCallbacks.AddToTail(func); }
	virtual void RemoveModeChangeCallback(ShaderModeChangeCallbackFunc_t func)
		{ m_ModeChangeCallbacks.FindAndRemove(func); }

	FORCEINLINE void InvokeModeChangeCallbacks(void)
	{
		int count = m_ModeChangeCallbacks.Count(), i;
		for (i = 0; i < count; ++i)
			m_ModeChangeCallbacks[i]();
	}
	static void *ShaderInterfaceFactory(const char *pInterfaceName, int *pReturnCode);

private:
	CUtlVector<ShaderModeChangeCallbackFunc_t> m_ModeChangeCallbacks;
};

extern CShaderDeviceMgrOES2 *g_pShaderDeviceMgrOES2;

#endif // !OES2_DEVMGR_H