//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 color format.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
#include "oes2_glext.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

unsigned int CShaderAPIOES2::ImageFormatToOESFormat(ImageFormat fmt) const
{
	const HardwareCaps_t &caps = HardwareConfig()->Caps();
	bool compressedTextures = ShaderUtil()->GetConfig().bCompressedTextures;

	switch (fmt)
	{
	case IMAGE_FORMAT_RGBA8888:
	case IMAGE_FORMAT_ABGR8888:
	case IMAGE_FORMAT_ARGB8888:
	case IMAGE_FORMAT_BGRA8888:
		return GL_RGBA | (GL_UNSIGNED_BYTE << 16);

	case IMAGE_FORMAT_BGRX8888:
	case IMAGE_FORMAT_RGB888:
	case IMAGE_FORMAT_BGR888:
		return GL_RGB | (GL_UNSIGNED_BYTE << 16);

	case IMAGE_FORMAT_BGR565:
	case IMAGE_FORMAT_BGRX5551:
	case IMAGE_FORMAT_RGB565:
		return GL_RGB | (GL_UNSIGNED_SHORT_5_6_5 << 16);

	case IMAGE_FORMAT_BGRA5551:
	case IMAGE_FORMAT_ABGR5551:
		return GL_RGBA | (GL_UNSIGNED_SHORT_5_5_5_1 << 16);

	case IMAGE_FORMAT_BGRA4444:
	case IMAGE_FORMAT_ABGR4444:
		return GL_RGBA | (GL_UNSIGNED_SHORT_4_4_4_4 << 16);

	case IMAGE_FORMAT_I8:
		return GL_LUMINANCE | (GL_UNSIGNED_BYTE << 16);

	case IMAGE_FORMAT_IA88:
		return GL_LUMINANCE_ALPHA | (GL_UNSIGNED_BYTE << 16);

	case IMAGE_FORMAT_A8:
		return GL_ALPHA | (GL_UNSIGNED_BYTE << 16);

	case IMAGE_FORMAT_DXT1:
		if (compressedTextures && caps.m_SupportsCompressedTextures)
			return GL_COMPRESSED_RGB_S3TC_DXT1_NV;
		return GL_RGB | (GL_UNSIGNED_BYTE << 16);
	case IMAGE_FORMAT_DXT1_ONEBITALPHA:
		if (compressedTextures && caps.m_SupportsCompressedTextures)
			return GL_COMPRESSED_RGBA_S3TC_DXT1_NV;
		return GL_RGBA | (GL_UNSIGNED_BYTE << 16);
	case IMAGE_FORMAT_DXT3:
		if (compressedTextures && (caps.m_SupportsCompressedTextures == COMPRESSED_TEXTURES_DXT))
			return GL_COMPRESSED_RGBA_S3TC_DXT3_NV;
		return GL_RGBA | (GL_UNSIGNED_BYTE << 16);
	case IMAGE_FORMAT_DXT5:
		if (compressedTextures && (caps.m_SupportsCompressedTextures == COMPRESSED_TEXTURES_DXT))
			return GL_COMPRESSED_RGBA_S3TC_DXT5_NV;
		return GL_RGBA | (GL_UNSIGNED_BYTE << 16);

	case IMAGE_FORMAT_UV88:
		return GL_LUMINANCE_ALPHA | (GL_UNSIGNED_BYTE << 16);
	case IMAGE_FORMAT_UVWQ8888:
	case IMAGE_FORMAT_UVLX8888:
		return GL_RGBA | (GL_UNSIGNED_BYTE << 16);

	// D0GHDR: 16-bit formats.
	}

	return 0;
}

ImageFormat CShaderAPIOES2::OESFormatToImageFormat(unsigned int fmt) const
{
	unsigned int pixelFormat = fmt & 0xffff;
	switch (fmt >> 16)
	{
	case GL_UNSIGNED_BYTE:
		switch (pixelFormat)
		{
		case GL_RGBA:
			return IMAGE_FORMAT_RGBA8888;
		case GL_RGB:
			return IMAGE_FORMAT_RGB888;
		case GL_ALPHA:
			return IMAGE_FORMAT_A8;
		case GL_LUMINANCE:
			return IMAGE_FORMAT_I8;
		case GL_LUMINANCE_ALPHA:
			return IMAGE_FORMAT_IA88;
		}
		break;
	case GL_UNSIGNED_SHORT_5_6_5:
		Assert(pixelFormat == GL_RGB);
		return IMAGE_FORMAT_BGR565;
	case GL_UNSIGNED_SHORT_4_4_4_4:
		Assert(pixelFormat == GL_RGBA);
		return IMAGE_FORMAT_ABGR4444;
	case GL_UNSIGNED_SHORT_5_5_5_1:
		Assert(pixelFormat == GL_RGBA);
		return IMAGE_FORMAT_ABGR5551;
	case 0: // Compressed.
		switch (pixelFormat)
		{
		case GL_COMPRESSED_RGB_S3TC_DXT1_NV:
			return IMAGE_FORMAT_DXT1;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_NV:
			return IMAGE_FORMAT_DXT1_ONEBITALPHA;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_NV:
			return IMAGE_FORMAT_DXT3;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_NV:
			return IMAGE_FORMAT_DXT5;
		}
		break;
	}
	return IMAGE_FORMAT_UNKNOWN;
}