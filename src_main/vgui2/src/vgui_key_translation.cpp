//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: 
//
//===========================================================================//

#if defined(_WIN32) && !defined(_X360)
#include <wtypes.h>
#include <winuser.h>
#include "xbox/xboxstubs.h"
#elif defined(__ANDROID__)
#include <android/keycodes.h>
#endif
#include "tier0/dbg.h"
#include "vgui_key_translation.h"
#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

#include "tier2/tier2.h"
#include "inputsystem/iinputsystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

vgui::KeyCode KeyCode_VirtualKeyToVGUI( int key )
{
	// Some tools load vgui for localization and never use input
	if ( !g_pInputSystem )
		return KEY_NONE;
	return g_pInputSystem->VirtualKeyToButtonCode( key );
}

int KeyCode_VGUIToVirtualKey( vgui::KeyCode code )
{
	// Some tools load vgui for localization and never use input
	if ( !g_pInputSystem )
#ifdef __ANDROID__
		return AKEYCODE_ENTER;
#else
		return VK_RETURN;
#endif

	return g_pInputSystem->ButtonCodeToVirtualKey( code );
}
