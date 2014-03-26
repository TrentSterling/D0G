//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 framebuffer and viewport functions.
//
//===========================================================================//
#include <cstd/string.h>
#include "oes2.h"
#include "oes2_gl.h"
#include "oes2_glext.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CShaderAPIOES2::ClearBuffers(bool bClearColor, bool bClearDepth, bool bClearStencil,
	int renderTargetWidth, int renderTargetHeight)
{
	if (ShaderUtil()->GetConfig().m_bSuppressRendering || IsDeactivated())
		return;

	FlushBufferedPrimitives();
	CommitSetViewports();
	CommitSetScissorRect();

	unsigned int mask = 0;
	if (bClearColor)
		mask |= GL_COLOR_BUFFER_BIT;
	if (bClearDepth)
		mask |= GL_DEPTH_BUFFER_BIT;
	if (bClearStencil)
		mask |= GL_STENCIL_BUFFER_BIT;

	if (!mask)
		return;

	const ShadowState_t *pShadow = m_TransitionTable.CurrentShadowState();
	bool maskColor = bClearColor && pShadow && (!pShadow->m_ColorWriteEnable || !pShadow->m_AlphaWriteEnable);

	if (bClearColor && m_ClearColorChanged)
	{
		glClearColor(
			m_DynamicState.m_ClearColor[0] * (1.0f / 255.0f),
			m_DynamicState.m_ClearColor[1] * (1.0f / 255.0f),
			m_DynamicState.m_ClearColor[2] * (1.0f / 255.0f),
			m_DynamicState.m_ClearColor[3] * (1.0f / 255.0f));
		m_ClearColorChanged = false;
	}

	if (maskColor)
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	if (bClearDepth)
	{
		bool bReverseDepth = ShaderUtil()->GetConfig().bReverseDepth;
		if (m_ClearReverseDepth != bReverseDepth)
		{
			m_ClearReverseDepth = bReverseDepth;
			glClearDepthf(bReverseDepth ? 0.0f : 1.0f);
		}
		if (!m_ZWriteEnable)
			glDepthMask(GL_TRUE);
	}

	if (bClearStencil)
	{
		if (m_ClearStencil)
		{
			m_ClearStencil = 0;
			glClearStencil(0);
		}
		if (m_DynamicState.m_StencilWriteMask != 0xffffffff)
			glStencilMask(0xffffffff);
	}

	glClear(mask);

	if (maskColor)
	{
		int colorMask = pShadow->m_ColorWriteEnable;
		glColorMask(colorMask, colorMask, colorMask, pShadow->m_AlphaWriteEnable);
	}
	if (bClearDepth && !m_ZWriteEnable)
		glDepthMask(GL_FALSE);
	if (bClearStencil && (m_DynamicState.m_StencilWriteMask != 0xffffffff))
		glStencilMask(m_DynamicState.m_StencilWriteMask);
}

void CShaderAPIOES2::ClearBuffersObeyStencil(bool bClearColor, bool bClearDepth)
{
	if ((!bClearColor && !bClearDepth) || IsDeactivated())
		return;
	FlushBufferedPrimitives();
	ShaderUtil()->DrawClearBufferQuad(
		m_DynamicState.m_ClearColor[0],
		m_DynamicState.m_ClearColor[1],
		m_DynamicState.m_ClearColor[2],
		m_DynamicState.m_ClearColor[3],
		bClearColor, bClearDepth);
}

void CShaderAPIOES2::ClearColor3ub(unsigned char r, unsigned char g, unsigned char b)
{
	if ((m_DynamicState.m_ClearColor[0] == r) &&
		(m_DynamicState.m_ClearColor[1] == g) &&
		(m_DynamicState.m_ClearColor[2] == b) &&
		(m_DynamicState.m_ClearColor[3] == 255))
		return;
	m_DynamicState.m_ClearColor[0] = r;
	m_DynamicState.m_ClearColor[1] = g;
	m_DynamicState.m_ClearColor[2] = b;
	m_DynamicState.m_ClearColor[3] = 255;
	m_ClearColorChanged = true;
}

void CShaderAPIOES2::ClearColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	if ((m_DynamicState.m_ClearColor[0] == r) &&
		(m_DynamicState.m_ClearColor[1] == g) &&
		(m_DynamicState.m_ClearColor[2] == b) &&
		(m_DynamicState.m_ClearColor[3] == a))
		return;
	m_DynamicState.m_ClearColor[0] = r;
	m_DynamicState.m_ClearColor[1] = g;
	m_DynamicState.m_ClearColor[2] = b;
	m_DynamicState.m_ClearColor[3] = a;
	m_ClearColorChanged = true;
}

void CShaderAPIOES2::ClearStencilBufferRectangle(int xmin, int ymin, int xmax, int ymax, int value)
{
	if (ShaderUtil()->GetConfig().m_bSuppressRendering || IsDeactivated())
		return;
	if (m_ClearStencil != value)
	{
		m_ClearStencil = value;
		glClearStencil(value);
	}
	if (m_ScissorEnabled)
	{
		SetScissorRect(xmin, ymin, xmax, ymax, true);
		Rect_t rect;
		bool changed = m_ScissorRectChanged;
		if (changed)
		{
			memcpy(&rect, &(m_DynamicState.m_ScissorRect), sizeof(Rect_t));
			CommitSetScissorRect();
		}
		glClear(GL_STENCIL_BUFFER_BIT);
		if (changed)
		{
			memcpy(&(m_DynamicState.m_ScissorRect), &rect, sizeof(Rect_t));
			m_ScissorRectChanged = true;
		}
	}
	else
	{
		SetScissorRect(xmin, ymin, xmax, ymax, true);
		CommitSetScissorRect();
		glClear(GL_STENCIL_BUFFER_BIT);
		m_ScissorEnabled = false;
	}
}

void CShaderAPIOES2::CopyRenderTargetToTextureEx(ShaderAPITextureHandle_t hTexture, int nRenderTargetID,
	Rect_t *pSrcRect, Rect_t *pDstRect)
{
	Assert(!nRenderTargetID); // No MRT.
	if (IsDeactivated())
		return;
	VPROF_BUDGET("CShaderAPIOES2::CopyRenderTargetToTexture", "Refraction overhead");

	if (!TextureIsAllocated(hTexture))
	{
		Assert(0);
		return;
	}
	Texture_t &tex = m_Textures[hTexture];
	if (ImageLoader::IsCompressed(tex.m_ImageFormat))
	{
		Assert(0);
		return;
	}

	int rtWidth, rtHeight;
	GetRenderTargetDimensions(rtWidth, rtHeight);

	Rect_t tempSrcRect, tempDstRect;
	if (!pSrcRect)
	{
		pSrcRect = &tempSrcRect;
		tempSrcRect.x = tempSrcRect.y = 0;
		tempSrcRect.width = rtWidth;
		tempSrcRect.height = rtHeight;
	}
	if (!pDstRect)
	{
		pDstRect = &tempDstRect;
		tempDstRect.x = tempDstRect.y = 0;
		tempDstRect.width = tex.m_Width;
		tempDstRect.height = tex.m_Height;
	}
	Assert((pSrcRect->width == pDstRect->width) && (pSrcRect->height == pDstRect->height)); // OES doesn't have StretchRect.

	BindTextureToTarget(hTexture);
	glCopyTexSubImage2D(
		(tex.m_Target == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D, 0,
		pDstRect->x, pDstRect->y, // Not flipping y because textures are upside-down.
		pSrcRect->x, rtHeight - pSrcRect->y - pSrcRect->height,
		pSrcRect->width, pSrcRect->height);
}

void CShaderAPIOES2::GetRenderTargetDimensions(int &width, int &height)
{
	ITexture *pTexture = GetRenderTargetEx(0);
	if (pTexture)
	{
		width = pTexture->GetActualWidth();
		height = pTexture->GetActualHeight();
	}
	else
	{
		GetBackBufferDimensions(width, height);
	}
}

int CShaderAPIOES2::GetViewports(ShaderViewport_t* pViewports, int nMax) const
{
	if (pViewports && (nMax > 0))
		memcpy(pViewports, &m_Viewport, sizeof(ShaderViewport_t));
	return 1;
}

void CShaderAPIOES2::PerformFullScreenStencilOperation(void)
{
	if (IsDeactivated())
		return;
	FlushBufferedPrimitives();
	ShaderUtil()->DrawClearBufferQuad(0, 0, 0, 0, false, false);
}

void CShaderAPIOES2::ReadPixels(int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat)
{
	if (IsDeactivated())
		return;
	int bufferWidth, bufferHeight;
	GetRenderTargetDimensions(bufferWidth, bufferHeight);
	y = bufferHeight - y - height;
	if (dstFormat == IMAGE_FORMAT_RGBA8888)
	{
		glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		ImageLoader::FlipImageVertically(data, data, width, height, IMAGE_FORMAT_RGBA8888);
	}
	else
	{
		unsigned char *rgba = new unsigned char[(width * height) << 2];
		glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
		ImageLoader::FlipImageVertically(rgba, rgba, width, height, IMAGE_FORMAT_RGBA8888);
		ShaderUtil()->ConvertImageFormat(rgba, IMAGE_FORMAT_RGBA8888, data, dstFormat, width, height);
		delete[] rgba;
	}
}

FORCEINLINE static float ReadPixelsLinInterp(float frac, float l, float r)
{
	return ((r - l) * frac) + l;
}

FORCEINLINE static unsigned int ReadPixelsInterp(float xFrac, float yFrac,
	unsigned int ul, unsigned int ur, unsigned int ll, unsigned int lr)
{
	float iu = ReadPixelsLinInterp(xFrac, (float)ul, (float)ur);
	float il = ReadPixelsLinInterp(xFrac, (float)ll, (float)lr);
	return (unsigned int)(ReadPixelsLinInterp(yFrac, iu, il));
}

static void ReadPixelsStretch(const uint32 *src, int srcWidth, int srcHeight, uint32 *dst, int dstWidth, int dstHeight)
{
	float xRatio = (float)srcWidth / (float)dstWidth;
	float yRatio = (float)srcHeight / (float)dstHeight;
	float sourceX, sourceY = 0, xFrac, yFrac;
	const uint32 *top, *bottom;
	int left, right, rowWidth;
	uint32 lt, rt, lb, rb;

	for (; dstHeight-- > 0; sourceY += yRatio)
	{
		yFrac = sourceY - floor(sourceY);
		top = src + ((int)sourceY) * srcWidth;
		bottom = top;
		if (((int)sourceY + 1) < srcHeight)
			bottom += srcWidth;
		for (rowWidth = dstWidth, sourceX = 0.0f; rowWidth-- > 0; sourceX += xRatio)
		{
			xFrac = sourceX - floor(sourceX);
			left = (int)sourceX;
			right = left + 1;
			if (right >= left)
				--right;
			lt = top[left];
			rt = top[right];
			lb = bottom[left];
			rb = bottom[right];
			*(dst++) =
				ReadPixelsInterp(xFrac, yFrac, lt & 0xff, rt & 0xff, lb & 0xff, rb & 0xff) |
				ReadPixelsInterp(xFrac, yFrac, (lt >> 8) & 0xff, (rt >> 8) & 0xff, (lb >> 8) & 0xff, (rb >> 8) & 0xff) |
				ReadPixelsInterp(xFrac, yFrac, (lt >> 16) & 0xff, (rt >> 16) & 0xff, (lb >> 16) & 0xff, (rb >> 16) & 0xff) |
				ReadPixelsInterp(xFrac, yFrac, lt >> 24, rt >> 24, lb >> 24, rb >> 24);
		}
	}
}

void CShaderAPIOES2::ReadPixels(Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *data, ImageFormat dstFormat, int nDstStride)
{
	int width = pSrcRect->width, height = pSrcRect->height;
	int dstWidth = pDstRect->width, dstHeight = pDstRect->height;
	int bufferWidth, bufferHeight;
	GetRenderTargetDimensions(bufferWidth, bufferHeight);
	int y = bufferHeight - pSrcRect->y - height;

	if ((dstFormat == IMAGE_FORMAT_RGBA8888) && (width == dstWidth) && (height == dstHeight))
	{
		glReadPixels(pSrcRect->x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		ImageLoader::FlipImageVertically(data, data, width, height, IMAGE_FORMAT_RGBA8888);
	}
	else
	{
		unsigned char *rgba = new unsigned char[(width * height) << 2];
		glReadPixels(pSrcRect->x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
		ImageLoader::FlipImageVertically(rgba, rgba, width, height, IMAGE_FORMAT_RGBA8888);
		if ((width == dstWidth) && (height == dstHeight))
		{
			ShaderUtil()->ConvertImageFormat(rgba, IMAGE_FORMAT_RGBA8888, data, dstFormat, dstWidth, dstHeight, nDstStride);
		}
		else
		{
			unsigned char *stretched = new unsigned char[(dstWidth * dstHeight) << 2];
			ReadPixelsStretch((const uint32 *)rgba, width, height, (uint32 *)stretched, dstWidth, dstHeight);
			ShaderUtil()->ConvertImageFormat(stretched, IMAGE_FORMAT_RGBA8888, data, dstFormat, dstWidth, dstHeight, nDstStride);
			delete[] stretched;
		}
		delete[] rgba;
	}
}

void CShaderAPIOES2::SetRenderTargetEx(int nRenderTargetID,
	ShaderAPITextureHandle_t colorTextureHandle, ShaderAPITextureHandle_t depthTextureHandle)
{
	Assert(!nRenderTargetID); // No MRT.
	if (IsDeactivated())
		return;
	if ((m_FramebufferColorTexture == colorTextureHandle) && (m_FramebufferDepthTexture == depthTextureHandle))
		return;

	FlushBufferedPrimitives();
	m_ViewportChanged = true;
	m_ScissorRectChanged = m_ScissorEnabled;

	if ((colorTextureHandle == SHADER_RENDERTARGET_BACKBUFFER) || (depthTextureHandle == SHADER_RENDERTARGET_DEPTHBUFFER))
	{
		// OES doesn't allow using default buffers for FBOs. Enforced by DoRenderTargetsNeedSeparateDepthBuffer.
		Assert((colorTextureHandle == SHADER_RENDERTARGET_BACKBUFFER)
			&& (depthTextureHandle == SHADER_RENDERTARGET_DEPTHBUFFER));
		m_FramebufferColorTexture = SHADER_RENDERTARGET_BACKBUFFER;
		m_FramebufferDepthTexture = SHADER_RENDERTARGET_DEPTHBUFFER;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	int hFB, firstfree = -1, found = -1;
	Framebuffer_t *pFB = NULL;
	for (hFB = m_Framebuffers.Head(); hFB != m_Framebuffers.InvalidIndex(); hFB = m_Framebuffers.Next(hFB))
	{
		pFB = &(m_Framebuffers[hFB]);
		if (!(pFB->m_Framebuffer))
		{
			firstfree = hFB;
			continue;
		}
		if ((pFB->m_ColorTexture == colorTextureHandle) && (pFB->m_DepthTexture == depthTextureHandle))
		{
			found = hFB;
			break;
		}
	}

	if (found < 0)
	{
		if (firstfree < 0)
			firstfree = m_Framebuffers.AddToTail();
		pFB = &(m_Framebuffers[firstfree]);
		pFB->m_Framebuffer = 0;
		glGenFramebuffers(1, &(pFB->m_Framebuffer));
		if (!(pFB->m_Framebuffer))
		{
			Warning("CShaderAPIOES2::SetRenderTargetEx: glGenFramebuffers failed: %s.\n", OESErrorString());
			return;
		}
		pFB->m_ColorTexture = colorTextureHandle;
		pFB->m_DepthTexture = depthTextureHandle;
		glBindFramebuffer(GL_FRAMEBUFFER, pFB->m_Framebuffer);

		Texture_t *tex;
		if (colorTextureHandle != SHADER_RENDERTARGET_NONE)
		{
			Assert(TextureIsAllocated(colorTextureHandle));
			tex = &(m_Textures[colorTextureHandle]);
			Assert(tex->m_Target == GL_TEXTURE_2D); // No point to render to cubemaps, and no arg for any side other than +X.
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->GetCurrentCopy(), 0);
		}
		if (depthTextureHandle != SHADER_RENDERTARGET_NONE)
		{
			Assert(TextureIsAllocated(depthTextureHandle));
			tex = &(m_Textures[depthTextureHandle]);
			Assert(tex->m_Flags & Texture_t::IS_DEPTH_STENCIL);
			if (tex->m_Flags & Texture_t::IS_DEPTH_STENCIL_TEXTURE)
			{
#ifdef SHADERAPIOES3
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL, GL_TEXTURE_2D, tex->m_DepthStencilSurface[0], 0);
#else
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex->m_DepthStencilSurface[0], 0);
				if (HardwareConfig()->Caps().m_DepthTextureFormat == GL_UNSIGNED_INT_24_8_OES)
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
						GL_TEXTURE_2D, tex->m_DepthStencilSurface[0], 0);
				}
				else
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
						GL_RENDERBUFFER, tex->m_DepthStencilSurface[1]);
				}
#endif
			}
			else
			{
#ifndef SHADERAPIOES3
				if (HardwareConfig()->Caps().m_DepthStencilFormat != GL_DEPTH24_STENCIL8_OES)
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
						GL_RENDERBUFFER, tex->m_DepthStencilSurface[0]);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
						GL_RENDERBUFFER, tex->m_DepthStencilSurface[1]);
				}
				else
#endif
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_OES,
						GL_RENDERBUFFER, tex->m_DepthStencilSurface[0]);
				}
			}
		}
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pFB->m_Framebuffer);
	}
}

void CShaderAPIOES2::SetScissorRect(const int nLeft, const int nTop, const int nRight, const int nBottom,
	const bool bEnableScissor)
{
	Assert((nLeft <= nRight) && (nTop <= nBottom));

	bool flush = m_ScissorEnabled != bEnableScissor;
	m_ScissorEnabled = bEnableScissor;

	if (bEnableScissor)
	{
		Rect_t newScissorRect;
		if (nRight > nLeft)
		{
			newScissorRect.x = nLeft;
			newScissorRect.width = nRight - nLeft;
		}
		else
		{
			newScissorRect.x = nRight;
			newScissorRect.width = nLeft - nRight;
		}
		if (nBottom > nTop)
		{
			newScissorRect.y = nTop;
			newScissorRect.height = nBottom - nTop;
		}
		else
		{
			newScissorRect.y = nBottom;
			newScissorRect.height = nTop - nBottom;
		}
		if (newScissorRect.x < 0)
			newScissorRect.x = 0;
		if (newScissorRect.y < 0)
			newScissorRect.y = 0;
		// Not clamping to width and height! But I think this is never exploited as a hack.
		if (memcmp(&(m_DynamicState.m_ScissorRect), &newScissorRect, sizeof(Rect_t)))
		{
			memcpy(&(m_DynamicState.m_ScissorRect), &newScissorRect, sizeof(Rect_t));
			m_ScissorRectChanged = true;
			flush = true;
		}
	}

	if (flush && !IsDeactivated())
		FlushBufferedPrimitives();
}

void CShaderAPIOES2::CommitSetScissorRect(void)
{
	if (!IsDeactivated())
		return;
	if (m_DynamicState.m_ScissorEnabled != m_ScissorEnabled)
	{
		if (m_ScissorEnabled)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
		m_DynamicState.m_ScissorEnabled = m_ScissorEnabled;
	}
	if (m_ScissorEnabled && m_ScissorRectChanged)
	{
		int bufferWidth, bufferHeight;
		GetRenderTargetDimensions(bufferWidth, bufferHeight);
		int width = m_DynamicState.m_ScissorRect.width, height = m_DynamicState.m_ScissorRect.height;
		if ((m_DynamicState.m_ScissorRect.x + width) > bufferWidth)
		{
			width = bufferWidth - m_DynamicState.m_ScissorRect.x;
			if (width < 0)
				width = 0;
		}
		if ((m_DynamicState.m_ScissorRect.y + height) > bufferHeight)
		{
			height = bufferHeight - m_DynamicState.m_ScissorRect.y;
			if (height < 0)
				height = 0;
		}
		glScissor(m_DynamicState.m_ScissorRect.x,
			bufferHeight - m_DynamicState.m_ScissorRect.y - height, // GL viewports and scissors are bottom-to-top.
			width, height);
		m_ScissorRectChanged = false;
	}
}

void CShaderAPIOES2::SetViewports(int nCount, const ShaderViewport_t *pViewports)
{
	Assert((nCount == 1) && (pViewports->m_nVersion == SHADER_VIEWPORT_VERSION));
	bool changed = false;
	// After a commit, desired==current, so an explicit check for changes in the current viewport is not required.
	if ((m_Viewport.m_nTopLeftX != pViewports->m_nTopLeftX) ||
		(m_Viewport.m_nTopLeftY != pViewports->m_nTopLeftY) ||
		(m_Viewport.m_nWidth != pViewports->m_nWidth) ||
		(m_Viewport.m_nHeight != pViewports->m_nHeight))
	{
		changed = true;
		m_ViewportChanged = true;
	}
	if ((m_Viewport.m_flMinZ != pViewports->m_flMinZ) ||
		(m_Viewport.m_flMaxZ != pViewports->m_flMaxZ))
	{
		changed = true;
		m_ViewportZChanged = true;
	}
	if (!changed)
		return;
	if (!IsDeactivated())
		FlushBufferedPrimitives();
	memcpy(&m_Viewport, pViewports, sizeof(ShaderViewport_t));
}

void CShaderAPIOES2::CommitSetViewports(void)
{
	if (!IsDeactivated())
		return;
	if (m_ViewportChanged)
	{
		int bufferWidth, bufferHeight;
		GetRenderTargetDimensions(bufferWidth, bufferHeight);
		int width = m_Viewport.m_nWidth, height = m_Viewport.m_nHeight;
		if ((m_Viewport.m_nTopLeftX + width) > bufferWidth)
		{
			width = bufferWidth - m_Viewport.m_nTopLeftX;
			if (width < 0)
				width = 0;
		}
		if ((m_Viewport.m_nTopLeftY + height) > bufferHeight)
		{
			height = bufferHeight - m_Viewport.m_nTopLeftY;
			if (height < 0)
				height = 0;
		}
		glViewport(m_Viewport.m_nTopLeftX,
			bufferHeight - m_Viewport.m_nTopLeftY - height, // GL viewports and scissors are bottom-to-top.
			width, height);
		m_ViewportChanged = false;
	}
	if (m_ViewportZChanged)
	{
		glDepthRangef(m_Viewport.m_flMinZ, m_Viewport.m_flMaxZ);
		m_ViewportZChanged = false;
	}
}