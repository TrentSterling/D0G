//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Android support for TrueType Fonts.
//
//===========================================================================//
#include <cstd/string.h>
#include <vgui/ISurface.h>
#include "vgui_surfacelib/FontManager.h"
#include "vgui_surfacelib/Win32Font.h"
#include "FontEffects.h"
#include FT_MODULE_H
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CEIL_FT_PIXEL(px) (((px) + 63) >> 6)
#define WIN32FONT_FLAGS_MASK (~(vgui::ISurface::FONTFLAG_UNDERLINE | vgui::ISurface::FONTFLAG_ANTIALIAS))

CWin32Font::CWin32Font(void) : m_ExtendedABCWidthsCache(256, 0, &ExtendedABCWidthsCacheLessFunc)
{
	m_szName = UTL_INVAL_SYMBOL;
	m_iTall = 0;
	m_iWeight = 400;
	m_iHeight = 0;
	m_iAscent = 0;
	m_iFlags = 0;
	m_iMaxCharWidth = 0;
	m_FTFace = NULL;
	m_bAntiAliased = true; // Stock Android fonts look ugly without anti-aliasing.
	m_bUnderlined = false;
	m_iBlur = 0;
	m_iScanLines = 0;
	m_bRotary = false;
	m_bAdditive = false;
	m_rgiBitmapSize[0] = m_rgiBitmapSize[1] = 0;
	m_ExtendedABCWidthsCache.EnsureCapacity(128);
}

CWin32Font::~CWin32Font()
{
	if (m_FTFace)
	{
		FT_Done_Face(m_FTFace);
		m_FTFace = NULL;
	}
}

bool CWin32Font::Create(const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags)
{
	FT_Library library = FontManager().GetFTLibrary();
	if (!library)
		return false;

	const unsigned char *file = FontManager().GetFontFile(windowsFontName);
	if (!file)
	{
		m_szName = UTL_INVAL_SYMBOL;
		return false;
	}

	FT_Face face;

	FT_Open_Args args;
	args.flags = FT_OPEN_MEMORY | FT_OPEN_DRIVER;
	args.memory_base = file + sizeof(int);
	args.memory_size = *((const int *)file);
	args.stream = NULL;
	args.driver = FT_Get_Module(library, "truetype");
	if (FT_Open_Face(library, &args, 0, &face))
	{
		m_szName = UTL_INVAL_SYMBOL;
		return false;
	}
	m_FTFace = face;

	m_szName = windowsFontName;
	m_iTall = tall;
	m_iFlags = flags & WIN32FONT_FLAGS_MASK;
	m_iScanLines = scanlines;
	m_iBlur = blur;
	m_iDropShadowOffset = (flags & vgui::ISurface::FONTFLAG_DROPSHADOW) ? 1 : 0;
	m_iOutlineSize = (flags & vgui::ISurface::FONTFLAG_OUTLINE) ? 1 : 0;
	m_bRotary = (flags & vgui::ISurface::FONTFLAG_ROTARY) ? 1 : 0;
	m_bAdditive = (flags & vgui::ISurface::FONTFLAG_ADDITIVE) ? 1 : 0;

	tall <<= 6;
	FT_Set_Char_Size(face, tall, tall, 72, 72);

	FT_Size_Metrics &metrics = face->size->metrics;
	m_iHeight = CEIL_FT_PIXEL(metrics.height) + m_iDropShadowOffset + (m_iOutlineSize << 1);
	m_iMaxCharWidth = CEIL_FT_PIXEL(metrics.max_advance);
	m_iAscent = CEIL_FT_PIXEL(metrics.ascender);
	m_iBaseline = CEIL_FT_PIXEL(metrics.height + metrics.descender);

	m_rgiBitmapSize[0] = m_iMaxCharWidth + (m_iOutlineSize << 1);
	m_rgiBitmapSize[1] = m_iHeight;

	return true;
}

void CWin32Font::GetCharRGBA(wchar_t ch, int rgbaWide, int rgbaTall, unsigned char *rgba)
{
	unsigned int glyphIndex = FT_Get_Char_Index(m_FTFace, ch);
	if (!glyphIndex || FT_Load_Glyph(m_FTFace, glyphIndex, FT_LOAD_RENDER))
		return;
	FT_GlyphSlot glyph = m_FTFace->glyph;
	FT_Bitmap &bitmap = glyph->bitmap;

	int srcX = 0;
	int dstX = glyph->bitmap_left;
	if (dstX < 0)
	{
		srcX = -dstX;
		dstX = 0;
	}
	int srcY = 0;
	int dstY = m_iBaseline - glyph->bitmap_top; 
	if (dstY < 0)
	{
		srcY = -dstY;
		dstY = 0;
	}

	dstX += m_iOutlineSize;
	dstY += m_iOutlineSize;

	int wide = bitmap.width - srcX;
	if ((dstX + wide) > rgbaWide)
		wide = rgbaWide - dstX;
	int tall = bitmap.rows - srcY;
	if ((dstY + tall) > rgbaTall)
		tall = rgbaTall - dstY;

	dstX <<= 2;
	rgbaWide <<= 2;
	unsigned char *srcRow = bitmap.buffer + srcY * bitmap.pitch, *src;
	unsigned char *dstRow = rgba + dstY * rgbaWide, *dst;
	int x;
	for (; tall-- > 0; srcRow += bitmap.pitch, dstRow += rgbaWide)
	{
		for (x = wide, src = srcRow + srcX, dst = dstRow + dstX; x-- > 0; ++src, dst += 4)
		{
			if (!(*src))
				continue;
			dst[0] = dst[1] = dst[2] = 255;
			dst[3] = *src;
		}	
	}
	rgbaWide >>= 2;

	ApplyDropShadowToTexture(rgbaWide, rgbaTall, rgba, m_iDropShadowOffset);
	ApplyOutlineToTexture(rgbaWide, rgbaTall, rgba, m_iOutlineSize);
	ApplyGaussianBlurToTexture(rgbaWide, rgbaTall, rgba, m_iBlur);
	ApplyScanlineEffectToTexture(rgbaWide, rgbaTall, rgba, m_iScanLines);
	ApplyRotaryEffectToTexture(rgbaWide, rgbaTall, rgba, m_bRotary);
}

bool CWin32Font::IsEqualTo(const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags)
{
	return !stricmp(windowsFontName, m_szName.String())
		&& (m_iTall == tall) && (m_iBlur == blur) && (m_iFlags == (flags & WIN32FONT_FLAGS_MASK));
}

bool CWin32Font::IsValid(void)
{
	return m_szName.IsValid() && m_szName.String()[0];
}

void CWin32Font::GetCharABCWidths(int ch, int &a, int &b, int &c)
{
	Assert(IsValid());

	abc_cache_t finder = { (wchar_t)ch };
	unsigned short i = m_ExtendedABCWidthsCache.Find(finder);
	if (m_ExtendedABCWidthsCache.IsValidIndex(i))
	{
		abc_cache_t &cache = m_ExtendedABCWidthsCache[i];
		a = cache.abc.a;
		b = cache.abc.b;
		c = cache.abc.c;
		return;
	}

	unsigned int glyphIndex = FT_Get_Char_Index(m_FTFace, ch);
	if (glyphIndex && !FT_Load_Glyph(m_FTFace, glyphIndex, FT_LOAD_DEFAULT))
	{
		FT_Glyph_Metrics &metrics = m_FTFace->glyph->metrics;
		a = CEIL_FT_PIXEL(metrics.horiBearingX);
		b = CEIL_FT_PIXEL(metrics.width);
		c = CEIL_FT_PIXEL(metrics.horiAdvance - metrics.horiBearingX - metrics.width);
	}
	else
	{
		a = c = 0;
		b = m_iMaxCharWidth;
	}

	a -= m_iBlur + m_iOutlineSize;
	b += ((m_iBlur + m_iOutlineSize) << 1) + m_iDropShadowOffset;
	c -= m_iBlur + m_iDropShadowOffset + m_iOutlineSize;

	finder.abc.a = a;
	finder.abc.b = b;
	finder.abc.c = c;
	m_ExtendedABCWidthsCache.Insert(finder);
}

int CWin32Font::GetHeight(void)
{
	Assert(IsValid());
	return m_iHeight;
}

int CWin32Font::GetAscent(void)
{
	Assert(IsValid());
	return m_iAscent;
}

int CWin32Font::GetMaxCharWidth(void)
{
	Assert(IsValid());
	return m_iMaxCharWidth;
}

int CWin32Font::GetFlags(void)
{
	return m_iFlags;
}

bool CWin32Font::ExtendedABCWidthsCacheLessFunc(const abc_cache_t &lhs, const abc_cache_t &rhs)
{
	return lhs.wch < rhs.wch;
}