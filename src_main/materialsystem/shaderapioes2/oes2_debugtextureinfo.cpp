//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 debug texture info.
//
//===========================================================================//
#include "oes2.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CShaderAPIOES2::EndFrame(void)
{
	// Export texture list.

	if (!m_EnableDebugTextureList || IsDeactivated())
		return;
	m_DebugDataExportFrame = m_CurrentFrame;
	if (m_pDebugTextureList)
		m_pDebugTextureList->deleteThis();
	KeyValues *list = new KeyValues("TextureList");
	m_pDebugTextureList = list;
	m_TextureMemoryUsedTotal = m_TextureMemoryUsedPicMip1 = m_TextureMemoryUsedPicMip2 = 0;

	ShaderAPITextureHandle_t hTexture;
	Texture_t *tex;
	int numBytes;
	KeyValues *pSubKey;

	for (hTexture = m_Textures.Head(); hTexture != m_Textures.InvalidIndex(); hTexture = m_Textures.Next(hTexture))
	{
		tex = &(m_Textures[hTexture]);
		if (!(tex->m_Flags & Texture_t::IS_ALLOCATED))
			continue;
		numBytes = tex->m_SizeBytes;
		m_TextureMemoryUsedTotal += numBytes;
		if ((tex->m_NumLevels > 1) && ((tex->m_Width > 4) || (tex->m_Height > 4) || (tex->m_Depth > 4)))
		{
			int topmipsize = ImageLoader::GetMemRequired(tex->m_Width, tex->m_Height, tex->m_Depth, tex->m_ImageFormat, false);
			numBytes -= topmipsize;
			m_TextureMemoryUsedPicMip1 += numBytes;
			if ((tex->m_Width > 8) || (tex->m_Height > 8) || (tex->m_Depth > 8))
				numBytes -= topmipsize >> (((tex->m_Width > 8) ? 1 : 0) + ((tex->m_Height > 8) ? 1 : 0) + ((tex->m_Depth > 8) ? 1 : 0));
			m_TextureMemoryUsedPicMip2 += numBytes;
		}
		else
		{
			m_TextureMemoryUsedPicMip1 += numBytes;
			m_TextureMemoryUsedPicMip2 += numBytes;
		}

		if (tex->m_LastBoundFrame != m_CurrentFrame)
		{
			if (!m_DebugGetAllTextures)
				continue;
			tex->m_TimesBoundThisFrame = 0;
		}

		pSubKey = list->CreateNewKey();
		pSubKey->SetString("Name", tex->m_DebugName.String());
		pSubKey->SetString("TexGroup", tex->m_TextureGroupName.String());
		pSubKey->SetInt("Size", tex->m_SizeBytes);
		pSubKey->SetString("Format", ImageLoader::GetName(tex->m_ImageFormat));
		pSubKey->SetInt("Width", tex->m_Width);
		pSubKey->SetInt("Height", tex->m_Height);
		pSubKey->SetInt("BindsMax", tex->m_TimesBoundMax);
		pSubKey->SetInt("BindsFrame", tex->m_TimesBoundThisFrame);
	}

	const char *buffers[] = { "DEPTHBUFFER", "FRONTBUFFER", "BACKBUFFER" };
	int width, height;
	GetBackBufferDimensions(width, height);
	int size = (width * height) << 2;
	int i;
	for (i = ARRAYSIZE(buffers); i--; )
	{
		pSubKey = list->CreateNewKey();
		pSubKey->SetString("Name", buffers[i]);
		pSubKey->SetString("TexGroup", TEXTURE_GROUP_RENDER_TARGET);
		pSubKey->SetInt("Size", size);
		pSubKey->SetString("Format", "32 bit buffer (hack)");
		pSubKey->SetInt("Width", width);
		pSubKey->SetInt("Height", height);
		pSubKey->SetInt("BindsMax", 1);
		pSubKey->SetInt("BindsFrame", 1);
	}
	VPROF_INCREMENT_GROUP_COUNTER("TexGroup_frame_" TEXTURE_GROUP_RENDER_TARGET, COUNTER_GROUP_TEXTURE_PER_FRAME,
		ARRAYSIZE(buffers) * size);
}

int CShaderAPIOES2::GetTextureMemoryUsed(IDebugTextureInfo::TextureMemoryType eTextureMemory)
{
	switch (eTextureMemory)
	{
	case MEMORY_BOUND_LAST_FRAME:
		return m_TextureMemoryUsedLastFrame;
	case MEMORY_TOTAL_LOADED:
		return m_TextureMemoryUsedTotal;
	case MEMORY_ESTIMATE_PICMIP_1:
		return m_TextureMemoryUsedPicMip1;
	case MEMORY_ESTIMATE_PICMIP_2:
		return m_TextureMemoryUsedPicMip2;
	}
	return 0;
}