//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 textures.
//
//===========================================================================//

#include <cstd/string.h>
#include "pixelwriter.h"
#include "oes2.h"
#include "oes2_gl.h"
#include "oes2_glext.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

ConVar mat_texture_limit("mat_texture_limit", "-1", FCVAR_NEVER_AS_STRING,
	"If this value is not -1, the material system will limit the amount of texture memory it uses in a frame." \
	" Useful for identifying performance cliffs. The value is in kilobytes.");

//-----------------------------------------------------------------------------
// Public interface.
//-----------------------------------------------------------------------------

void CShaderAPIOES2::BindStandardTexture(Sampler_t sampler, StandardTextureId_t id)
{
	if (m_StdTextureHandles[id] != INVALID_SHADERAPI_TEXTURE_HANDLE)
		BindTexture(sampler, m_StdTextureHandles[id]);
	else
		ShaderUtil()->BindStandardTexture(sampler, id);
}

void CShaderAPIOES2::BindTexture(Sampler_t sampler, ShaderAPITextureHandle_t hTexture)
{
	if (m_BoundTextures[sampler] == hTexture)
		return;
	if ((hTexture == INVALID_SHADERAPI_TEXTURE_HANDLE) || WouldBeOverTextureLimit(hTexture))
		return; // No need to unbind in GL - just continue using the previous texture.
	SetActiveTexture(sampler);
	BindTextureToTarget(hTexture);
	Texture_t &tex = m_Textures[hTexture];
	if (tex.m_LastBoundFrame != m_CurrentFrame)
	{
		tex.m_LastBoundFrame = m_CurrentFrame;
		tex.m_TimesBoundThisFrame = 0;
		if (tex.m_pTextureGroupCounterFrame)
			*(tex.m_pTextureGroupCounterFrame) += tex.m_SizeBytes;
		m_TextureMemoryUsedLastFrame += tex.m_SizeBytes;
	}
	if (!m_DebugTexturesRendering && (++tex.m_TimesBoundThisFrame > tex.m_TimesBoundMax))
		tex.m_TimesBoundMax = tex.m_TimesBoundThisFrame;
}

ShaderAPITextureHandle_t CShaderAPIOES2::CreateDepthTexture(ImageFormat renderTargetFormat, int width, int height,
	const char *pDebugName, bool bTexture)
{
	ShaderAPITextureHandle_t hTexture;
	CreateTextureHandles(&hTexture, 1);
	Texture_t &tex = m_Textures[hTexture];

	tex.m_MagFilter = tex.m_MinFilter = GL_NEAREST;
	tex.m_Target = GL_TEXTURE_2D;
	tex.m_NumLevels = 1;
	tex.m_NumCopies = 1;
	tex.m_Width = width;
	tex.m_Height = height;
	tex.m_Depth = 1;
	tex.m_Flags = Texture_t::IS_ALLOCATED;
	// The only time where it's used to calculate size is ExportTextureList for mipmaps, but depth textures have no mipmaps.
	tex.m_ImageFormat = IMAGE_FORMAT_UNKNOWN;
	tex.m_DebugName = pDebugName;
	tex.m_DepthStencilSurface[0] = 0;
#ifndef SHADERAPIOES3
	tex.m_DepthStencilSurface[1] = 0;
#endif

	unsigned int error, handles[2];
	handles[0] = 0;

	unsigned int depthTextureFormat = HardwareConfig()->Caps().m_DepthTextureFormat;
	if (bTexture && depthTextureFormat)
	{
		tex.m_Flags |= Texture_t::IS_DEPTH_STENCIL_TEXTURE;
#ifndef SHADERAPIOES3
		if (depthTextureFormat != GL_UNSIGNED_INT_24_8_OES)
		{
			Assert((depthTextureFormat == GL_UNSIGNED_SHORT) || (depthTextureFormat == GL_UNSIGNED_INT));
			tex.m_SizeBytes = ImageLoader::GetMemRequired(width, height, 1, IMAGE_FORMAT_I8, false);
			if (depthTextureFormat == GL_UNSIGNED_INT)
				tex.m_SizeBytes += ImageLoader::GetMemRequired(width, height, 1, IMAGE_FORMAT_RGBA8888, false);
			else
				tex.m_SizeBytes += ImageLoader::GetMemRequired(width, height, 1, IMAGE_FORMAT_RGB565, false);

			glGenTextures(1, handles);
			if (!handles[0])
			{
				Warning("CShaderAPIOES2::CreateDepthTexture: glGenTextures failed: %s.\n", OESErrorString());
				return hTexture;
			}
			tex.m_DepthStencilSurface[0] = handles[0];
			m_BoundTextures[m_ActiveTexture] = INVALID_SHADERAPI_TEXTURE_HANDLE;
			glBindTexture(GL_TEXTURE_2D, handles[0]);
			SetupTexParameters(tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, depthTextureFormat, NULL);

			handles[0] = 0;
			glGenRenderbuffers(1, handles);
			if (!handles[0])
			{
				glDeleteTextures(1, tex.m_DepthStencilSurface);
				tex.m_DepthStencilSurface[0] = 0;
				Warning("CShaderAPIOES2::CreateDepthTexture: glGenRenderbuffers(1) failed: %s.\n", OESErrorString());
				return hTexture;
			}
			tex.m_DepthStencilSurface[1] = handles[0];
			glBindRenderbuffer(GL_RENDERBUFFER, handles[0]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
		}
		else
#endif
		{
			tex.m_SizeBytes = ImageLoader::GetMemRequired(width, height, 1, IMAGE_FORMAT_NV_DST24, false);
			glGenTextures(1, handles);
			if (!handles[0])
			{
				Warning("CShaderAPIOES2::CreateDepthTexture: glGenTextures failed: %s.\n", OESErrorString());
				return hTexture;
			}
			tex.m_DepthStencilSurface[0] = handles[0];
			m_BoundTextures[m_ActiveTexture] = INVALID_SHADERAPI_TEXTURE_HANDLE;
			glBindTexture(GL_TEXTURE_2D, handles[0]);
			SetupTexParameters(tex);
#ifdef SHADERAPIOES3
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
#else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL_OES, width, height, 0, GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES, NULL);
#endif
		}
	}
	else
	{
		tex.m_Flags |= Texture_t::IS_DEPTH_STENCIL_BUFFER;
#ifndef SHADERAPIOES3
		unsigned int format = HardwareConfig()->Caps().m_DepthStencilFormat;
		if (format != GL_DEPTH24_STENCIL8_OES)
		{
			Assert((format == GL_DEPTH_COMPONENT16) || (format == GL_DEPTH_COMPONENT24_OES));
			tex.m_SizeBytes = ImageLoader::GetMemRequired(width, height, 1, IMAGE_FORMAT_I8, false);
			if (format == GL_DEPTH_COMPONENT24_OES)
				tex.m_SizeBytes += ImageLoader::GetMemRequired(width, height, 1, IMAGE_FORMAT_RGBA8888, false);
			else
				tex.m_SizeBytes += ImageLoader::GetMemRequired(width, height, 1, IMAGE_FORMAT_RGB565, false);

			handles[1] = 0;
			glGenRenderbuffers(2, handles);
			if (!(handles[0]) || !(handles[1]))
			{
				if (handles[0])
					glDeleteRenderbuffers(1, handles);
				Warning("CShaderAPIOES2::CreateDepthTexture: glGenRenderbuffers(2) failed: %s.\n", OESErrorString());
				return hTexture;
			}
			tex.m_DepthStencilSurface[0] = handles[0];
			tex.m_DepthStencilSurface[1] = handles[1];
			glBindRenderbuffer(GL_RENDERBUFFER, handles[0]);
			glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, handles[1]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
		}
		else
#endif
		{
			tex.m_SizeBytes = ImageLoader::GetMemRequired(width, height, 1, IMAGE_FORMAT_NV_DST24, false);
			glGenRenderbuffers(1, handles);
			if (!handles[0])
			{
				Warning("CShaderAPIOES2::CreateDepthTexture: glGenRenderbuffers(1) failed: %s.\n", OESErrorString());
				return hTexture;
			}
			tex.m_DepthStencilSurface[0] = handles[0];
			glBindRenderbuffer(GL_RENDERBUFFER, handles[0]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, width, height);
		}
	}

	SetupTextureGroup(hTexture, TEXTURE_GROUP_RENDER_TARGET);
	return hTexture;
}

ShaderAPITextureHandle_t CShaderAPIOES2::CreateTexture(int width, int height, int depth,
		ImageFormat dstImageFormat, int numMipLevels, int numCopies, int creationFlags,
		const char *pDebugName, const char *pTextureGroupName)
{
	ShaderAPITextureHandle_t handle;
	CreateTextures(&handle, 1, width, height, depth, dstImageFormat, numMipLevels, numCopies, creationFlags,
		pDebugName, pTextureGroupName);
	return handle;
}

void CShaderAPIOES2::CreateTextures(ShaderAPITextureHandle_t *pHandles, int count, int width, int height, int depth,
	ImageFormat dstImageFormat, int numMipLevels, int numCopies, int creationFlags,
	const char *pDebugName, const char *pTextureGroupName)
{
	unsigned int pxFormat = ImageFormatToOESFormat(dstImageFormat);
	if (!pxFormat)
	{
		Assert(0);
		memset(pHandles, 0, sizeof(ShaderAPITextureHandle_t) * count);
		return;
	}
	dstImageFormat = OESFormatToImageFormat(pxFormat);
	unsigned int pxType = pxFormat >> 16;
	pxFormat &= 0xffff;

	if (creationFlags & TEXTURE_CREATE_RENDERTARGET)
		numMipLevels = 1;
	else if (numMipLevels < 1)
		numMipLevels = 1;
	unsigned int minFilter = (numMipLevels != 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;

	unsigned int target, firstFace;
	int numFaces;
	if (depth < 1)
		depth = 1;
	int sizeBytes = ImageLoader::GetMemRequired(width, height, depth, dstImageFormat, numMipLevels > 1);
	if (depth != 1)
	{
		Assert((width <= HardwareConfig()->Caps().m_MaxTextureDepth)
			&& (height <= HardwareConfig()->Caps().m_MaxTextureDepth)
			&& (depth <= HardwareConfig()->Caps().m_MaxTextureDepth) && pxType);
		target = GL_TEXTURE_3D_OES;
	}
	else if (creationFlags & TEXTURE_CREATE_CUBEMAP)
	{
		Assert((width <= HardwareConfig()->Caps().m_MaxCubeMapTextureSize)
			&& (height <= HardwareConfig()->Caps().m_MaxCubeMapTextureSize));
		target = GL_TEXTURE_CUBE_MAP;
		firstFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		numFaces = 6;
		sizeBytes *= 6;
	}
	else
	{
		Assert((creationFlags & TEXTURE_CREATE_RENDERTARGET)
			|| ((width <= HardwareConfig()->Caps().m_MaxTextureSize)
			&& (height <= HardwareConfig()->Caps().m_MaxTextureSize)));
		Assert(!(creationFlags & TEXTURE_CREATE_RENDERTARGET)
			|| ((width <= HardwareConfig()->Caps().m_MaxRenderTargetSize)
			&& (height <= HardwareConfig()->Caps().m_MaxRenderTargetSize)));
		target = firstFace = GL_TEXTURE_2D;
		numFaces = 1;
	}

	if (numCopies < 1)
		numCopies = 1;

	m_BoundTextures[m_ActiveTexture] = INVALID_SHADERAPI_TEXTURE_HANDLE; // Going to bind copies of the new textures.

	CreateTextureHandles(pHandles, count);
	Texture_t *tex;
	int frame = 0, copy, face, level;
	unsigned int *copies;
	for (frame = 0; frame < count; ++frame)
	{
		tex = &(m_Textures[pHandles[frame]]);
		tex->m_MagFilter = GL_LINEAR;
		tex->m_MinFilter = minFilter;
		tex->m_Target = target;
		tex->m_NumLevels = numMipLevels;
		tex->m_NumCopies = numCopies;
		tex->m_DebugName = pDebugName;
		tex->m_SizeBytes = sizeBytes;
		tex->m_Width = width;
		tex->m_Height = height;
		tex->m_Depth = depth;
		tex->m_Flags = Texture_t::IS_ALLOCATED;
		tex->m_ImageFormat = dstImageFormat;
		SetupTextureGroup(pHandles[frame], pTextureGroupName);
		if (numCopies > 1)
		{
			copies = new unsigned int[numCopies];
			tex->m_Textures = copies;
			memset(copies, 0, sizeof(unsigned int) * numCopies);
			glGenTextures(numCopies, copies);
			for (copy = numCopies; copy-- > 0; )
			{
				glBindTexture(target, copies[copy]);
				SetupTexParameters(*tex);
				if (depth != 1)
				{
					for (level = 0; level < numMipLevels; ++level)
					{
						glTexImage3D(GL_TEXTURE_3D_OES, level, pxFormat,
							max(width >> level, 1), max(height >> level, 1), max(depth >> level, 1),
							0, pxFormat, pxType, NULL);
					}
				}
				else if (pxType) // Unlikely to do SubImage2D on compressed textures.
				{
					for (face = numFaces; face-- > 0; )
					{
						for (level = 0; level < numMipLevels; ++level)
						{
							glTexImage2D(firstFace + face, level, pxFormat,
								max(width >> level, 1), max(height >> level, 1), 0, pxFormat, pxType, NULL);
						}
					}
				}
			}
		}
		else
		{
			tex->m_Texture = 0;
			glGenTextures(1, &(tex->m_Texture));
			glBindTexture(target, tex->m_Texture);
			SetupTexParameters(*tex);
			if (depth != 1)
			{
				for (level = 0; level < numMipLevels; ++level)
					glTexImage3D(GL_TEXTURE_3D_OES, level, pxFormat, width, height, depth, 0, pxFormat, pxType, NULL);
			}
			else if (pxType)
			{
				for (face = numFaces; face-- > 0; )
				{
					for (level = 0; level < numMipLevels; ++level)
					{
						glTexImage2D(firstFace + face, level, pxFormat,
							max(width >> level, 1), max(height >> level, 1), 0, pxFormat, pxType, NULL);
					}
				}
			}
		}
	}
}

void CShaderAPIOES2::DeleteOESTexture(Texture_t &tex)
{
	if (tex.m_Flags & Texture_t::IS_DEPTH_STENCIL_TEXTURE)
	{
		glDeleteTextures(1, tex.m_DepthStencilSurface);
#ifndef SHADERAPIOES3
		if (HardwareConfig()->Caps().m_DepthTextureFormat != GL_UNSIGNED_INT_24_8_OES)
			glDeleteRenderbuffers(1, &(tex.m_DepthStencilSurface[1]));
#endif
	}
	else
	{
		if (tex.m_Flags & Texture_t::IS_DEPTH_STENCIL_BUFFER)
		{
			glDeleteRenderbuffers((HardwareConfig()->Caps().m_DepthStencilFormat == GL_DEPTH24_STENCIL8_OES) ? 1 : 2,
				tex.m_DepthStencilSurface);
		}
		else
		{
			if (tex.m_NumCopies > 1)
			{
				glDeleteTextures(tex.m_NumCopies, tex.m_Textures);
				delete[] tex.m_Textures;
			}
			else
			{
				glDeleteTextures(1, &(tex.m_Texture));
			}
		}
	}
}

void CShaderAPIOES2::DeleteTexture(ShaderAPITextureHandle_t hTexture)
{
	if (!TextureIsAllocated(hTexture))
	{
		Assert(0);
		return;
	}
	Texture_t &tex = m_Textures[hTexture];

	if (tex.m_pTextureGroupCounterGlobal)
	{
		*(tex.m_pTextureGroupCounterGlobal) -= tex.m_SizeBytes;
		Assert(*(tex.m_pTextureGroupCounterGlobal) >= 0);
	}

	int i;
	for (i = 0; i < TEXTURE_MAX_STD_TEXTURES; ++i)
	{
		if (m_StdTextureHandles[i] == hTexture)
			m_StdTextureHandles[i] = INVALID_SHADERAPI_TEXTURE_HANDLE;
	}

	UnbindTexture(hTexture);
	if ((m_FramebufferColorTexture == hTexture) || (m_FramebufferDepthTexture == hTexture))
		SetRenderTarget(SHADER_RENDERTARGET_BACKBUFFER, SHADER_RENDERTARGET_DEPTHBUFFER);
	int hFB;
	Framebuffer_t *pFB;
	for (hFB = m_Framebuffers.Head(); hFB != m_Framebuffers.InvalidIndex(); hFB = m_Framebuffers.Next(hFB))
	{
		pFB = &(m_Framebuffers[hFB]);
		if (!(pFB->m_Framebuffer))
			continue;
		if ((pFB->m_ColorTexture == hTexture) || (pFB->m_DepthTexture == hTexture))
		{
			if (m_DeviceActive)
				glDeleteFramebuffers(1, &(pFB->m_Framebuffer));
			pFB->m_Framebuffer = 0;
		}
	}

	if (m_DeviceActive)
		DeleteOESTexture(tex);
	tex.m_Flags = 0;
}

bool CShaderAPIOES2::IsTexture(ShaderAPITextureHandle_t hTexture)
{
	if (!TextureIsAllocated(hTexture))
		return false;
	Texture_t &tex = m_Textures[hTexture];
	if (tex.m_Flags & Texture_t::IS_DEPTH_STENCIL)
		return tex.m_DepthStencilSurface[0] != 0;
	return tex.GetCurrentCopy() != 0;
}

void CShaderAPIOES2::ModifyTexture(ShaderAPITextureHandle_t hTexture)
{
	Assert(TextureIsAllocated(hTexture));
	m_ModifyTextureHandle = hTexture;
	Texture_t &tex = m_Textures[hTexture];
	if (tex.m_NumCopies > 1)
		tex.m_SwitchNeeded = 1;
}

void CShaderAPIOES2::SetAnisotropicLevel(int nAnisotropyLevel)
{
	if (ShaderUtil()) // This function is called from the main ctor when ShaderUtil is not available yet.
		ShaderUtil()->NoteAnisotropicLevel(nAnisotropyLevel);
	int maxAnisotropy = HardwareConfig()->Caps().m_MaxAnisotropy;
	if (maxAnisotropy <= 1)
		return;
	if ((nAnisotropyLevel > maxAnisotropy) || (nAnisotropyLevel <= 1))
		nAnisotropyLevel = max(2, min(8, maxAnisotropy >> 2));
	if (m_AnisotropicLevel == nAnisotropyLevel)
		return;
	m_AnisotropicLevel = nAnisotropyLevel;

	if (IsDeactivated())
		return;
	ShaderAPITextureHandle_t hTexture;
	Texture_t *tex;
	for (hTexture = m_Textures.Head(); hTexture != m_Textures.InvalidIndex(); hTexture = m_Textures.Next(hTexture))
	{
		if (!TextureIsAllocated(hTexture))
			continue;
		tex = &(m_Textures[hTexture]);
		if (!(tex->m_AnisotropicFilter))
			continue;
		BindTextureToTarget(hTexture);
		TexParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, m_AnisotropicLevel);
	}
}

void CShaderAPIOES2::TexImage2D(int level, int cubeFaceID, ImageFormat dstFormat, int zOffset,
	int width, int height, ImageFormat srcFormat, bool bSrcIsTiled, void *imageData)
{
	Assert(imageData);
	if (!TextureIsAllocated(m_ModifyTextureHandle))
	{
		Assert(0);
		return;
	}
	Texture_t &tex = m_Textures[m_ModifyTextureHandle];
	// Texture_t must contain valid data.
	Assert((width == max(tex.m_Width >> level, 1)) && (height == max(tex.m_Height >> level, 1)));
	if ((level >= tex.m_NumLevels) || (zOffset >= max(tex.m_Depth >> level, 1)))
	{
		Assert(0);
		return;
	}
	if (tex.m_SwitchNeeded)
	{
		AdvanceCurrentCopy(m_ModifyTextureHandle);
		tex.m_SwitchNeeded = 0;
	}
	if (IsDeactivated())
		return;

	unsigned int oesFmt = ImageFormatToOESFormat(dstFormat);
	if (!oesFmt)
	{
		Assert(0);
		return;
	}
	ImageFormat fmt = OESFormatToImageFormat(oesFmt);
	Assert(fmt == tex.m_ImageFormat);
	unsigned int pixelType = oesFmt >> 16;
	int dstSize = ImageLoader::GetMemRequired(width, height, 1, fmt, false);

	unsigned char *data = (unsigned char *)imageData, *converted = NULL;
	if (dstFormat != fmt)
	{
		if (dstSize <= 16384) // A small buffer enough for the smallest RGBA8888 texture in OES2.
		{
			unsigned char *smallConverted = (unsigned char *)(stackalloc(dstSize));
			if (!(ShaderUtil()->ConvertImageFormat(data, srcFormat, smallConverted, fmt, width, height)))
			{
				Assert(0);
				return;
			}
			data = smallConverted;
		}
		else
		{
			converted = new unsigned char[dstSize];
			if (!(ShaderUtil()->ConvertImageFormat(data, srcFormat, converted, fmt, width, height)))
			{
				Assert(0);
				delete[] converted;
				return;
			}
			data = converted;
		}
	}

	// 3D target is checked later.
	unsigned int target = (tex.m_Target == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFaceID : GL_TEXTURE_2D;
	BindTextureToTarget(m_ModifyTextureHandle);
	if (pixelType)
	{
		SetUnpackAlignment(fmt, width);
		if (tex.m_Depth > 1)
			glTexSubImage3D(GL_TEXTURE_3D_OES, level, 0, 0, zOffset, width, height, 1, oesFmt & 0xffff, pixelType, data);
		else
			glTexSubImage2D(target, level, 0, 0, width, height, oesFmt & 0xffff, pixelType, data);
	}
	else
	{
		// Not SubImage2D because of issues with small mipmaps. 3D textures cannot be compressed at this moment.
		glCompressedTexImage2D(target, level, oesFmt, width, height, 0, dstSize, data);
	}

	if (converted)
		delete[] converted;
}

void CShaderAPIOES2::TexSubImage2D(int level, int cubeFaceID, int xOffset, int yOffset, int zOffset,
	int width, int height, ImageFormat srcFormat, int srcStride, bool bSrcIsTiled, void *imageData)
{
	Assert(imageData);
	if (!TextureIsAllocated(m_ModifyTextureHandle))
	{
		Assert(0);
		return;
	}
	Texture_t &tex = m_Textures[m_ModifyTextureHandle];
	if (level >= tex.m_NumLevels)
		return;
	if (tex.m_SwitchNeeded)
	{
		AdvanceCurrentCopy(m_ModifyTextureHandle);
		tex.m_SwitchNeeded = 0;
	}
	if (IsDeactivated())
		return;

	ImageFormat fmt = tex.m_ImageFormat;
	unsigned int oesFmt = ImageFormatToOESFormat(fmt);
	int dstSize = ImageLoader::GetMemRequired(width, height, 1, fmt, false);

	unsigned char *data = (unsigned char *)imageData, *converted = NULL;
	// The stride equation must match the one in CVTFTexture::RowSizeInBytes.
	if ((fmt != srcFormat) || (srcStride != (ImageLoader::SizeInBytes(fmt) * width)))
	{
		if (dstSize <= 16384) // A small buffer enough for the smallest RGBA8888 texture in OES2.
		{
			unsigned char *smallConverted = (unsigned char *)(stackalloc(dstSize));
			if (!(ShaderUtil()->ConvertImageFormat(data, srcFormat, smallConverted, fmt, width, height, srcStride)))
			{
				Assert(0);
				return;
			}
			data = smallConverted;
		}
		else
		{
			converted = new unsigned char[dstSize];
			if (!(ShaderUtil()->ConvertImageFormat(data, srcFormat, converted, fmt, width, height, srcStride)))
			{
				Assert(0);
				delete[] converted;
				return;
			}
			data = converted;
		}
	}

	// 3D target is checked later.
	unsigned int target = (tex.m_Target == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFaceID : GL_TEXTURE_2D;
	unsigned int pixelType = oesFmt >> 16;
	BindTextureToTarget(m_ModifyTextureHandle);
	if (pixelType)
	{
		SetUnpackAlignment(fmt, width);
		// yOffset must not be mirrored here - shaders should take care of flipping!
		if (tex.m_Depth > 1)
			glTexSubImage3D(GL_TEXTURE_3D_OES, level, xOffset, yOffset, zOffset, width, height, 1, oesFmt & 0xffff, pixelType, data);
		else
			glTexSubImage2D(target, level, xOffset, yOffset, width, height, oesFmt & 0xffff, pixelType, data);
	}
	else
	{
		// 3D textures cannot be compressed at this moment.
		glCompressedTexSubImage2D(target, level, xOffset, yOffset, width, height, oesFmt, dstSize, data);
	}

	if (converted)
		delete[] converted;
}

bool CShaderAPIOES2::TexLock(int level, int cubeFaceID, int xOffset, int yOffset,
	int width, int height, CPixelWriter &writer)
{
	if (!TextureIsAllocated(m_ModifyTextureHandle))
	{
		Assert(0);
		return false;
	}
	Texture_t &tex = m_Textures[m_ModifyTextureHandle];
	if (level >= tex.m_NumLevels)
		return false;
	if (tex.m_SwitchNeeded)
	{
		AdvanceCurrentCopy(m_ModifyTextureHandle);
		tex.m_SwitchNeeded = 0;
	}
	ImageFormat fmt = tex.m_ImageFormat;
	m_TexLockLevel = level;
	m_TexLockCubeFaceID = cubeFaceID;
	m_TexLockXOffset = xOffset;
	m_TexLockYOffset = yOffset;
	m_TexLockWidth = width;
	m_TexLockHeight = height;
	m_pTexLockBits = new unsigned char[ImageLoader::GetMemRequired(width, height, 1, fmt, false)];
	writer.SetPixelMemory(fmt, m_pTexLockBits, ImageLoader::SizeInBytes(fmt) * width);
	return true;
}

void CShaderAPIOES2::TexUnlock(void)
{
	ImageFormat fmt = m_Textures[m_ModifyTextureHandle].m_ImageFormat;
	TexSubImage2D(m_TexLockLevel, m_TexLockCubeFaceID, m_TexLockXOffset, m_TexLockYOffset, 1,
		m_TexLockWidth, m_TexLockHeight, fmt, ImageLoader::SizeInBytes(fmt) * m_TexLockWidth, false, m_pTexLockBits);
	delete[] m_pTexLockBits;
}

void CShaderAPIOES2::TexMagFilter(ShaderTexFilterMode_t texFilterMode)
{
	Texture_t *tex = BindModifyTexture();
	if (!tex)
		return;

	int previous = tex->m_MagFilter;
	bool anisotropic = false;
	switch (texFilterMode)
	{
	case SHADER_TEXFILTERMODE_NEAREST:
		tex->m_MagFilter = GL_NEAREST;
		break;
	case SHADER_TEXFILTERMODE_LINEAR:
		tex->m_MagFilter = GL_LINEAR;
		break;
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_NEAREST:
		Warning("CShaderAPIOES2::TexMagFilter: SHADER_TEXFILTERMODE_NEAREST_MIPMAP_NEAREST is invalid\n");
		return;
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_NEAREST:
		Warning("CShaderAPIOES2::TexMagFilter: SHADER_TEXFILTERMODE_LINEAR_MIPMAP_NEAREST is invalid\n");
		return;
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_LINEAR:
		Warning("CShaderAPIOES2::TexMagFilter: SHADER_TEXFILTERMODE_NEAREST_MIPMAP_LINEAR is invalid\n");
		return;
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR:
		Warning("CShaderAPIOES2::TexMagFilter: SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR is invalid\n");
		return;
	case SHADER_TEXFILTERMODE_ANISOTROPIC:
		tex->m_MagFilter = GL_LINEAR;
		anisotropic = true;
		break;
	default:
		Warning("CShaderAPIOES2::TexMagFilter: Unknown texFilterMode\n");
		return;
	}
	if (!IsDeactivated() && (tex->m_MagFilter != previous))
		TexParameter(GL_TEXTURE_MAG_FILTER, tex->m_MagFilter);

	if (HardwareConfig()->Caps().m_MaxAnisotropy > 1)
	{
		if (anisotropic)
		{
			if (!IsDeactivated() && !(tex->m_AnisotropicFilter))
				TexParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, m_AnisotropicLevel);
			tex->m_AnisotropicFilter |= 2;
		}
		else
		{
			if (!IsDeactivated() && (tex->m_AnisotropicFilter == 2))
				TexParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
			tex->m_AnisotropicFilter &= (unsigned char)(~2);
		}
	}
}

void CShaderAPIOES2::TexMinFilter(ShaderTexFilterMode_t texFilterMode)
{
	Texture_t *tex = BindModifyTexture();
	if (!tex)
		return;

	int previous = tex->m_MinFilter;
	bool anisotropic = false;
	switch (texFilterMode)
	{
	case SHADER_TEXFILTERMODE_NEAREST:
		tex->m_MinFilter = GL_NEAREST;
		break;
	case SHADER_TEXFILTERMODE_LINEAR:
		tex->m_MinFilter = GL_LINEAR;
		break;
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_NEAREST:
		tex->m_MinFilter = (tex->m_NumLevels > 1) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
		break;
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_NEAREST:
		tex->m_MinFilter = (tex->m_NumLevels > 1) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
		break;
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_LINEAR:
		tex->m_MinFilter = (tex->m_NumLevels > 1) ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST;
		break;
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR:
		tex->m_MinFilter = (tex->m_NumLevels > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
		break;
	case SHADER_TEXFILTERMODE_ANISOTROPIC:
		tex->m_MinFilter = (tex->m_NumLevels > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
		anisotropic = true;
		break;
	default:
		Warning("CShaderAPIOES2::TexMinFilter: Unknown texFilterMode\n");
		return;
	}
	if (!IsDeactivated() && (tex->m_MinFilter != previous))
		TexParameter(GL_TEXTURE_MIN_FILTER, tex->m_MinFilter);

	if (HardwareConfig()->Caps().m_MaxAnisotropy > 1)
	{
		if (anisotropic)
		{
			if (!IsDeactivated() && !(tex->m_AnisotropicFilter))
				TexParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, m_AnisotropicLevel);
			tex->m_AnisotropicFilter |= 1;
		}
		else
		{
			if (!IsDeactivated() && (tex->m_AnisotropicFilter == 1))
				TexParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
			tex->m_AnisotropicFilter &= (unsigned char)(~1);
		}
	}
}

void CShaderAPIOES2::TexWrap(ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode)
{
	Texture_t *tex = BindModifyTexture();
	if (!tex)
		return;

	int address;
	switch (wrapMode)
	{
	case SHADER_TEXWRAPMODE_CLAMP:
		address = GL_CLAMP_TO_EDGE;
		break;
	case SHADER_TEXWRAPMODE_REPEAT:
		address = GL_REPEAT;
		break;
	case SHADER_TEXWRAPMODE_BORDER:
		address = (HardwareConfig()->Caps().m_SupportsBorderColor) ? GL_CLAMP_TO_BORDER_NV : GL_CLAMP_TO_EDGE;
		break;
	default:
		address = GL_CLAMP_TO_EDGE;
		Warning("CShaderAPIOES2::TexWrap: unknown wrapMode\n");
		break;
	}

	switch (coord)
	{
	case SHADER_TEXCOORD_S:
		if (tex->m_UTexWrap != address)
		{
			tex->m_UTexWrap = address;
			if (!IsDeactivated())
				TexParameter(GL_TEXTURE_WRAP_S, address);
		}
		break;
	case SHADER_TEXCOORD_T:
		if (tex->m_VTexWrap != address)
		{
			tex->m_VTexWrap = address;
			if (!IsDeactivated())
				TexParameter(GL_TEXTURE_WRAP_T, address);
		}
		break;
	case SHADER_TEXCOORD_U:
		if ((tex->m_Depth > 1) && (tex->m_WTexWrap != address))
		{
			tex->m_WTexWrap = address;
			if (!IsDeactivated())
				TexParameter(GL_TEXTURE_WRAP_R_OES, address);
		}
		break;
	default:
		Warning("CShaderAPIOES2::TexWrap: unknown coord\n");
		break;
	}
}

//-----------------------------------------------------------------------------
// Private API functions.
//-----------------------------------------------------------------------------

void CShaderAPIOES2::BindTextureToTarget(ShaderAPITextureHandle_t hTexture)
{
	Assert(TextureIsAllocated(hTexture));
	if (m_BoundTextures[m_ActiveTexture] == hTexture)
		return;
	m_BoundTextures[m_ActiveTexture] = hTexture;
	if (!IsDeactivated())
	{
		Texture_t &tex = m_Textures[hTexture];
		glBindTexture(tex.m_Target, tex.GetCurrentCopy());
	}
}

void CShaderAPIOES2::CreateTextureHandles(ShaderAPITextureHandle_t *handles, int count)
{
	if (count <= 0)
		return;
	MEM_ALLOC_CREDIT();
	int idxCreating = 0;
	ShaderAPITextureHandle_t hTexture;
	for (hTexture = m_Textures.Head(); hTexture != m_Textures.InvalidIndex(); hTexture = m_Textures.Next(hTexture))
	{
		if (m_Textures[hTexture].m_Flags & Texture_t::IS_ALLOCATED)
			continue;
		handles[idxCreating++] = hTexture;
		m_Textures[hTexture].Defaults();
		if (idxCreating >= count)
			return;
	}
	while (idxCreating < count)
	{
		hTexture = m_Textures.AddToTail();
		handles[idxCreating++] = hTexture;
		m_Textures[hTexture].Defaults();
	}
}

void CShaderAPIOES2::SetActiveTexture(Sampler_t sampler)
{
	if (m_ActiveTexture == sampler)
		return;
	if (!IsDeactivated())
		glActiveTexture(GL_TEXTURE0 + sampler);
	m_ActiveTexture = sampler;
}

void CShaderAPIOES2::SetUnpackAlignment(ImageFormat fmt, int width)
{
	Assert(!IsDeactivated()); // TexImage2D and TexSubImage2D pre-check, must not be called from anywhere else.
	int size = ImageLoader::GetMemRequired(width, 1, 1, fmt, false);
	int alignment;
	if (size & 1)
		alignment = 1;
	else if (size & 3)
		alignment = 2;
	else
		alignment = 4;
	if (m_UnpackAlignment != alignment)
	{
		m_UnpackAlignment = alignment;
		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	}
}

void CShaderAPIOES2::SetupTextureGroup(ShaderAPITextureHandle_t hTexture, const char *pTextureGroupName)
{
	Texture_t &tex = m_Textures[hTexture];
	if (!pTextureGroupName || !(*pTextureGroupName))
		pTextureGroupName = TEXTURE_GROUP_UNACCOUNTED;
	tex.m_TextureGroupName = pTextureGroupName;

#ifdef VPROF_ENABLED
	char counterName[256];
	Q_snprintf(counterName, sizeof(counterName), "TexGroup_global_%s", pTextureGroupName);
	tex.m_pTextureGroupCounterGlobal =
		g_VProfCurrentProfile.FindOrCreateCounter(counterName, COUNTER_GROUP_TEXTURE_GLOBAL);
	Q_snprintf(counterName, sizeof(counterName), "TexGroup_frame_%s", pTextureGroupName);
	tex.m_pTextureGroupCounterFrame =
		g_VProfCurrentProfile.FindOrCreateCounter(counterName, COUNTER_GROUP_TEXTURE_PER_FRAME);

	if (tex.m_pTextureGroupCounterGlobal)
		*(tex.m_pTextureGroupCounterGlobal) += tex.m_SizeBytes;
#endif
}

void CShaderAPIOES2::SetupTexParameters(const Texture_t &tex)
{
	if (tex.m_UTexWrap != GL_REPEAT)
		glTexParameteri(tex.m_Target, GL_TEXTURE_WRAP_S, tex.m_UTexWrap);
	if (tex.m_VTexWrap != GL_REPEAT)
		glTexParameteri(tex.m_Target, GL_TEXTURE_WRAP_T, tex.m_VTexWrap);
	if ((tex.m_Depth > 1) && (tex.m_WTexWrap != GL_REPEAT))
		glTexParameteri(tex.m_Target, GL_TEXTURE_WRAP_R_OES, tex.m_WTexWrap);
	if (tex.m_MagFilter != GL_LINEAR)
		glTexParameteri(tex.m_Target, GL_TEXTURE_MAG_FILTER, tex.m_MagFilter);
	if (tex.m_MinFilter != GL_NEAREST_MIPMAP_LINEAR)
		glTexParameteri(tex.m_Target, GL_TEXTURE_MIN_FILTER, tex.m_MinFilter);
}

void CShaderAPIOES2::TexParameter(unsigned int pname, int param)
{
	Assert(!IsDeactivated()); // This will be called a lot in SetAnisotropicLevel.
	Assert(TextureIsAllocated(m_BoundTextures[m_ActiveTexture]));
	const Texture_t &tex = m_Textures[m_BoundTextures[m_ActiveTexture]];
	Assert(!(tex.m_Flags & Texture_t::IS_DEPTH_STENCIL)); // No need to filter depth at the moment.
	if (tex.m_NumCopies > 1)
	{
		int i, current = tex.m_CurrentCopy;
		for (i = 0; i < tex.m_NumCopies; ++i)
		{
			glTexParameteri(tex.m_Target, pname, param); // The first copy is already bound.
			if (++current >= tex.m_NumCopies)
				current = 0;
			glBindTexture(tex.m_Target, tex.m_Textures[current]); // The last bind will rebind the previously bound copy.
		}
	}
	else
	{
		glTexParameteri(tex.m_Target, pname, param); // The texture is already bound.
	}
}

void CShaderAPIOES2::UnbindTexture(ShaderAPITextureHandle_t hTexture)
{
	int i;
	for (i = HardwareConfig()->Caps().m_NumCombinedSamplers; i-- > 0; )
	{
		if (m_BoundTextures[i] == hTexture)
			m_BoundTextures[i] = INVALID_SHADERAPI_TEXTURE_HANDLE;
	}
}

bool CShaderAPIOES2::WouldBeOverTextureLimit(ShaderAPITextureHandle_t hTexture)
{
	int limit = mat_texture_limit.GetInt();
	if (limit < 0)
		return false;
	Texture_t &tex = m_Textures[hTexture];
	if (tex.m_LastBoundFrame == m_CurrentFrame)
		return false;
	return (m_TextureMemoryUsedLastFrame + tex.m_SizeBytes) > (limit << 10);
}