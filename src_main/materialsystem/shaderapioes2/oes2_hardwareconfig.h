//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 hardware configuration info.
//
//===========================================================================//
#ifndef OES2_HARDWARECONFIG_H
#define OES2_HARDWARECONFIG_H

#include "../materialsystem/IHardwareConfigInternal.h"

#define MAX_NUM_LIGHTS 2
#define MAX_NUM_PIXEL_LIGHT_CONSTANTS 4

enum CompressedTextureState_t
{
	COMPRESSED_TEXTURES_OFF,
	COMPRESSED_TEXTURES_DXT1, // EXT_texture_compression_dxt1, doesn't support DXT3 and DXT5.
	COMPRESSED_TEXTURES_DXT
};

struct HardwareCaps_t : public MaterialAdapterInfo_t
{
	int m_MaxAnisotropy;
	int m_MaxCubeMapTextureSize;
	int m_NumCombinedSamplers;
	int m_MaxRenderTargetSize;
	int m_MaxTextureDepth;
	int m_MaxTextureSize;
	int m_NumPixelShaderConstants;
	int m_NumPixelShaderInputs;
	int m_NumPixelShaderSamplers;
	int m_NumVertexShaderConstants;
	int m_NumVertexShaderInputs;
	int m_NumVertexShaderSamplers;

	unsigned int m_DepthStencilFormat;
	unsigned int m_DepthTextureFormat;
	unsigned int m_OcclusionQuery;
	char m_ShaderDLL[32];
	CompressedTextureState_t m_SupportsCompressedTextures;

	bool m_SupportsBorderColor : 1;
	bool m_SupportsFragmentHighPrecision : 1;

	HardwareCaps_t(void) // Minimum values for OES2 or OES3.
	{
		m_pDriverName[0] = m_VendorID = m_DeviceID = m_SubSysID = m_Revision = 0;
		m_nDXSupportLevel = m_nMaxDXSupportLevel = 90; // 81 has too simple shaders.
		m_nDriverVersionLow = 0;

		m_MaxAnisotropy = 1;
		m_ShaderDLL[0] = 0;
		m_SupportsBorderColor = false;
		m_SupportsCompressedTextures = COMPRESSED_TEXTURES_OFF;

#ifdef SHADERAPIOES3
		m_nDriverVersionHigh = 3;

		m_MaxCubeMapTextureSize = 2048;
		m_MaxRenderTargetSize = 2048;
		m_MaxTextureDepth = 256;
		m_MaxTextureSize = 2048;
		m_NumPixelShaderConstants = 224;
		m_NumPixelShaderInputs = 15;
		m_NumPixelShaderSamplers = 16;
		m_NumVertexShaderConstants = 256;
		m_NumVertexShaderInputs = 16;
		m_NumVertexShaderSamplers = OES2_SHADER_MAX_VERTEX_SAMPLERS; // 16, but Source exposes less (4 in The Orange Box).
		m_NumCombinedSamplers = 16 + OES2_SHADER_MAX_VERTEX_SAMPLERS;
		m_DepthStencilFormat = 0x88F0; // DEPTH24_STENCIL8
		m_DepthTextureFormat = 0x84FA; // UNSIGNED_INT_24_8
		m_OcclusionQuery = 0x8D6A; // ANY_SAMPLES_PASSED_CONSERVATIVE
		m_SupportsFragmentHighPrecision = true;
#else
		m_nDriverVersionHigh = 2;

		m_MaxCubeMapTextureSize = 16;
		m_MaxRenderTargetSize = 256;
		m_MaxTextureDepth = 1;
		m_MaxTextureSize = 256; // Minimum for text rendering.
		m_NumPixelShaderConstants = 16;
		m_NumPixelShaderInputs = 8;
		m_NumPixelShaderSamplers = 8;
		m_NumVertexShaderConstants = 128;
		m_NumVertexShaderInputs = 8;
		m_NumVertexShaderSamplers = 0;
		m_NumCombinedSamplers = 8 + 0;
		m_DepthStencilFormat = 0x81A5; // DEPTH_COMPONENT16
		m_DepthTextureFormat = 0;
		m_OcclusionQuery = 0;
		m_SupportsFragmentHighPrecision = false;
#endif
	}
};

class CHardwareConfig : public IHardwareConfigInternal
{
public:
	virtual bool HasDestAlphaBuffer(void) const { return true; }
	virtual bool HasStencilBuffer(void) const { return true; }
	virtual int GetFrameBufferColorDepth(void) const { return 4; }
	virtual int GetSamplerCount(void) const { return m_Caps.m_NumPixelShaderSamplers; }
	virtual bool HasSetDeviceGammaRamp(void) const { return false; }
	virtual bool SupportsCompressedTextures(void) const
		{ return m_Caps.m_SupportsCompressedTextures != COMPRESSED_TEXTURES_OFF; }
	// Normal decompression code is very big.
	// I don't really want to inline it in a branch or compile another shader combo because of 20-30 megabytes.
	virtual VertexCompressionType_t SupportsCompressedVertices(void) const { return VERTEX_COMPRESSION_NONE; }
	virtual bool SupportsNormalMapCompression(void) const { return false; } // Not in OES, disabled in DX.
	virtual bool SupportsVertexAndPixelShaders(void) const { return true; }
	virtual bool SupportsPixelShaders_1_4(void) const { return true; }
	virtual bool SupportsPixelShaders_2_0(void) const { return true; }
	virtual bool SupportsVertexShaders_2_0(void) const { return true; }
	virtual int MaximumAnisotropicLevel(void) const { return m_Caps.m_MaxAnisotropy; }
	virtual int MaxTextureWidth(void) const { return m_Caps.m_MaxTextureSize; }
	virtual int MaxTextureHeight(void) const { return m_Caps.m_MaxTextureSize; }
	virtual int TextureMemorySize(void) const { return 64 * 1024 * 1024; } // Fake because memory is usually shared.
	virtual bool SupportsOverbright(void) const { return false; } // FF.
	virtual bool SupportsCubeMaps(void) const { return true; }
	virtual bool SupportsMipmappedCubemaps(void) const { return true; }
	virtual bool SupportsNonPow2Textures(void) const { return true; } // No wrap and mipmaps!
	virtual int GetTextureStageCount(void) const { return 0; }
	virtual int NumVertexShaderConstants(void) const { return m_Caps.m_NumVertexShaderConstants; }
	virtual int NumPixelShaderConstants(void) const { return m_Caps.m_NumPixelShaderConstants; }
	virtual int MaxNumLights(void) const { return MAX_NUM_LIGHTS; }
	virtual bool SupportsHardwareLighting(void) const { return false; }
	virtual int MaxBlendMatrices(void) const { return 0; } // FF.
	virtual int MaxBlendMatrixIndices(void) const { return 0; } // FF.
	virtual int MaxTextureAspectRatio(void) const { return m_Caps.m_MaxTextureSize; }
	virtual int MaxVertexShaderBlendMatrices(void) const { return 16; } // DX9 models have flex meshes, no need to support them yet.
	virtual int MaxUserClipPlanes(void) const { return 0; }
	virtual bool UseFastClipping(void) const { return true; }
	virtual int GetDXSupportLevel(void) const { return 90; }
	virtual const char *GetShaderDLLName(void) const { return m_Caps.m_ShaderDLL[0] ? m_Caps.m_ShaderDLL : "DEFAULT"; }
	virtual bool ReadPixelsFromFrontBuffer(void) const { return false; } // Always false in DX8.
	virtual bool PreferDynamicTextures(void) const { return false; }
	virtual bool SupportsHDR(void) const { return false; } // D0GHDR.
	virtual bool HasProjectedBumpEnv(void) const { return false; } // FF.
	virtual bool SupportsSpheremapping(void) const { return false; } // FF.
	virtual bool NeedsAAClamp(void) const { return false; } // Always false in DX8.
	virtual bool NeedsATICentroidHack(void) const { return false; }
	virtual bool SupportsColorOnSecondStream(void) const { return true; }
	virtual bool SupportsStaticPlusDynamicLighting(void) const { return true; }
	virtual bool PreferReducedFillrate(void) const { return ShaderUtil()->GetConfig().ReduceFillrate(); }
	virtual int GetMaxDXSupportLevel(void) const { return 90; } // == GetDXSupportLevel.
	virtual bool SpecifiesFogColorInLinearSpace(void) const { return false; } // D0GSRGB.
	virtual bool SupportsSRGB(void) const { return false; }
	virtual bool IsAAEnabled(void) const { return false; } // FXAA is used instead.
	virtual int GetVertexTextureCount(void) const { return m_Caps.m_NumVertexShaderSamplers; }
	virtual int GetMaxVertexTextureDimension(void) const { return 0; }
	virtual int MaxTextureDepth(void) const { return m_Caps.m_MaxTextureDepth; }
	virtual HDRType_t GetHDRType(void) const { return HDR_TYPE_NONE; } // D0GHDR.
	virtual HDRType_t GetHardwareHDRType(void) const { return HDR_TYPE_NONE; }
	virtual bool SupportsPixelShaders_2_b(void) const { return false; }
	virtual bool SupportsStreamOffset(void) const { return true; }
	virtual int StencilBufferBits(void) const { return 8; }
	virtual int MaxViewports(void) const { return 1; }
	virtual void OverrideStreamOffsetSupport(bool bOverrideEnabled, bool bEnableSupport) {} // Used only by ShaderAPITest.
	virtual int GetShadowFilterMode(void) const { return 0; }
	virtual int NeedsShaderSRGBConversion(void) const { return 0; } // D0GSRGB.
	virtual bool UsesSRGBCorrectBlending(void) const { return false; }
	virtual bool SupportsShaderModel_3_0(void) const { return false; }
	virtual bool HasFastVertexTextures(void) const { return false; } // Mainly for HWM, which isn't supported yet.
	virtual int MaxHWMorphBatchCount(void) const { return 0; }
	virtual bool ActuallySupportsPixelShaders_2_b(void) const { return false; }
	virtual bool SupportsHDRMode(HDRType_t nHDRMode) const { return nHDRMode == HDR_TYPE_NONE; } // D0GHDR.
	virtual bool GetHDREnabled(void) const { return false; }
	virtual void SetHDREnabled(bool bEnable) {}
	virtual bool SupportsBorderColor(void) const { return m_Caps.m_SupportsBorderColor; }
	virtual bool SupportsFetch4(void) const { return false; }
	virtual const char *GetHWSpecificShaderDLLName(void) const { return m_Caps.m_ShaderDLL[0] ? m_Caps.m_ShaderDLL : NULL; }

	virtual int MaxCubeMapTextureSize(void) const { return m_Caps.m_MaxCubeMapTextureSize; }
	virtual int NumPixelShaderInputs(void) const { return m_Caps.m_NumPixelShaderInputs; }
	virtual int MaxRenderTargetSize(void) const { return m_Caps.m_MaxRenderTargetSize; }

	const HardwareCaps_t &Caps(void) { return m_Caps; }
	bool CapsDetermined(void) const { return m_CapsDetermined; }
	void DetermineHardwareCaps(void);
	void DetermineHardwareCapsFromPbuffer(void);

protected:
	HardwareCaps_t m_Caps;
	bool m_CapsDetermined; // A GL context is required to get caps.
};

extern CHardwareConfig *g_pHardwareConfig;
FORCEINLINE CHardwareConfig *HardwareConfig(void) { return g_pHardwareConfig; }

#endif // !OES2_HARDWARECONFIG_H