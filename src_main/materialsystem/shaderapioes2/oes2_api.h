//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 shader API interface.
//
//===========================================================================//
#ifndef OES2_API_H
#define OES2_API_H

#include <cstd/string.h>
#include "materialsystem/idebugtextureinfo.h"
#include "renderparm.h"
#include "tier1/checksum_crc.h"
#include "tier1/KeyValues.h"
#include "tier1/utlbuffer.h"
#include "tier1/utldict.h"
#include "tier1/utllinkedlist.h"
#include "tier1/utlstack.h"

enum TransformType_t
{
	TRANSFORM_IS_GENERAL, // Moved to 0 from 2 to avoid immediates.
	TRANSFORM_IS_IDENTITY,
	TRANSFORM_IS_CAMERA_TO_WORLD
};

enum VertexShaderLightTypes_t
{
	LIGHT_NONE = -1,
	LIGHT_SPOT,
	LIGHT_POINT,
	LIGHT_DIRECTIONAL,
	LIGHT_STATIC,
	LIGHT_AMBIENTCUBE
};

struct DynamicState_t
{
	// Transform state.
	TransformType_t m_TransformType[NUM_MATRIX_MODES];

	// Light.
	LightDesc_t m_LightDescs[MAX_NUM_LIGHTS];
	bool m_LightEnable[MAX_NUM_LIGHTS];
	Vector4D m_AmbientLightCube[6];
	VertexShaderLightTypes_t m_LightType[MAX_NUM_LIGHTS];
	Vector m_LightingOrigin;
	int m_NumLights;

	// Clear color.
	unsigned char m_ClearColor[4];

	// Fog.
	float m_PixelFogColor[3];
	float m_FogStart;
	float m_FogEnd;
	float m_FogZ;
	float m_FogMaxDensity;

	// User clip planes.
	float m_HeightClipZ;
	MaterialHeightClipMode_t m_HeightClipMode;
	bool m_FastClipEnabled;
	bool m_FastClipPlaneChanged;
	VPlane m_FastClipPlane;
	bool m_UserClipTransformOverride;
	VMatrix m_UserClipTransform;

	// Cull mode.
	unsigned int m_FrontFace;
	bool m_CullEnabled;

	// Skinning.
	int m_NumBones;

	// Render state: stencil.
	bool m_StencilEnabled;
	unsigned int m_StencilFailOperation;
	unsigned int m_StencilZFailOperation;
	unsigned int m_StencilPassOperation;
	unsigned int m_StencilCompareFunction;
	int m_StencilReferenceValue;
	unsigned int m_StencilTestMask;
	unsigned int m_StencilWriteMask;

	// Render state: depth bias.
	bool m_DepthBiasEnabled;

	// Scissor test.
	bool m_ScissorEnabled;
	Rect_t m_ScissorRect;
};

struct Framebuffer_t
{
	ShaderAPITextureHandle_t m_ColorTexture;
	ShaderAPITextureHandle_t m_DepthTexture;
	unsigned int m_Framebuffer;
};

enum StreamStateOp_t // Vertex attribute stream opcodes. s_StreamStateFunctionTable of MeshMgr depends on this.
{
	// Stream 0.
	STREAM_STATE_OP_POSITION,
	STREAM_STATE_OP_BONE_WEIGHT,
	STREAM_STATE_OP_BONE_INDEX,
	STREAM_STATE_OP_NORMAL,
	STREAM_STATE_OP_COLOR,
	STREAM_STATE_OP_TEXCOORD0,
	STREAM_STATE_OP_TEXCOORD1,
	STREAM_STATE_OP_TEXCOORD2,
	STREAM_STATE_OP_TEXCOORD3,
	STREAM_STATE_OP_TEXCOORD4,
	STREAM_STATE_OP_TEXCOORD5,
	STREAM_STATE_OP_TEXCOORD6,
	STREAM_STATE_OP_TEXCOORD7,
	STREAM_STATE_OP_TANGENT_S,
	STREAM_STATE_OP_TANGENT_T,
	STREAM_STATE_OP_USERDATA,
	// Stream 0/1.
	STREAM_STATE_OP_SPECULAR,
	// Stream 2.
	STREAM_STATE_OP_POSITION_FLEX,
	STREAM_STATE_OP_NORMAL_FLEX,

	STREAM_STATE_OP_COUNT
};

struct ShaderProgram_t
{
	CRC32_t m_Name; // CRC32 hash of the name string.
	int m_Combo; // Shader-specific combo information for identification.

	unsigned int m_Program; // The GL program handle. 0 means the shader is unallocated/deleted.
	unsigned int m_VertexShader; // The GL vertex shader handle.
	unsigned int m_PixelShader; // The GL pixel shader handle.

	struct StandardConstant_t // Initialized with memset(-1).
	{
		int m_Location; // The GL uniform location.
		// If both don't match m_StandardConstUpdate, reupload the constant.
		int m_UpdateFrame; // The frame when the last update occured.
		int m_Update; // The last update number of this constant this frame.
	} m_StandardConstants[OES2_SHADER_CONST_COUNT];

	int m_Constants[OES2_SHADER_MAX_CONSTANTS]; // Shader-specific constant locations.

	// Attribute locations are not explicitly stored and are computed from the vertex usage instead.
	int m_VertexInputs[OES2_SHADER_INPUT_COUNT]; // Vertex attribute locations.
	VertexFormat_t m_VertexUsage; // The format of the vertex shader input.

	StreamStateOp_t m_StreamOps[STREAM_STATE_OP_COUNT]; // Vertex attribute stream opcodes.
	int m_StreamOpCount;

	int m_RefCount; // The number of materials that are using this program.

	void Defaults(void)
	{
		m_Program = 0;
		memset(m_StandardConstants, 0xff, sizeof(m_StandardConstants));
		memset(m_Constants, 0xff, sizeof(m_Constants));
		memset(m_VertexInputs, 0xff, sizeof(m_VertexInputs));
		m_VertexUsage = VERTEX_FORMAT_INVALID;
	}

	void SetStreamOps(void)
	{
		Assert(m_VertexUsage != VERTEX_FORMAT_INVALID);
		m_StreamOpCount = 0;

		// Stream 0.
		if (m_VertexUsage & VERTEX_POSITION)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_POSITION;
		if (m_VertexUsage & VERTEX_BONE_WEIGHT_MASK)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_BONE_WEIGHT;
		if (m_VertexUsage & VERTEX_BONE_INDEX)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_BONE_INDEX;
		if (m_VertexUsage & VERTEX_NORMAL)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_NORMAL;
		if (m_VertexUsage & VERTEX_COLOR)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_COLOR;
		if (m_VertexUsage & VERTEX_TANGENT_S)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_TANGENT_S;
		if (m_VertexUsage & VERTEX_TANGENT_T)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_TANGENT_T;
		if (m_VertexUsage & USER_DATA_SIZE_MASK)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_USERDATA;
		int i;
		for (i = VERTEX_MAX_TEXTURE_COORDINATES; i--; )
		{
			if (m_VertexUsage & VERTEX_TEXCOORD_MASK(i))
				m_StreamOps[m_StreamOpCount++] = (StreamStateOp_t)(((int)STREAM_STATE_OP_TEXCOORD0) + i);
		}
		// Stream 0/1.
		if (m_VertexUsage & VERTEX_SPECULAR)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_SPECULAR;
		// Stream 2.
		if (m_VertexInputs[OES2_SHADER_INPUT_POSITION_FLEX] >= 0)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_POSITION_FLEX;
		if (m_VertexInputs[OES2_SHADER_INPUT_NORMAL_FLEX] >= 0)
			m_StreamOps[m_StreamOpCount++] = STREAM_STATE_OP_NORMAL_FLEX;
	}
};

struct Texture_t
{
	// GLenums.
	int m_UTexWrap;
	int m_VTexWrap;
	int m_WTexWrap;
	int m_MagFilter;
	int m_MinFilter;
	unsigned int m_Target; // 2D or cubemap.

	unsigned char m_NumLevels;
	unsigned char m_NumCopies;
	unsigned char m_CurrentCopy;
	unsigned char m_SwitchNeeded : 4;
	unsigned char m_AnisotropicFilter : 4; // Bitfield: 1 - min enabled, 2 - mag enabled.

	CUtlSymbol m_DebugName;
	CUtlSymbol m_TextureGroupName;
	int *m_pTextureGroupCounterGlobal;
	int *m_pTextureGroupCounterFrame;

	int m_SizeBytes;
	int m_LastBoundFrame;
	int m_TimesBoundMax;
	int m_TimesBoundThisFrame;

	enum
	{
		IS_ALLOCATED = 0x01,
		IS_DEPTH_STENCIL_BUFFER = 0x02,
		IS_DEPTH_STENCIL_TEXTURE = 0x04,
		IS_DEPTH_STENCIL = IS_DEPTH_STENCIL_BUFFER | IS_DEPTH_STENCIL_TEXTURE
	};

	short m_Width;
	short m_Height;
	short m_Depth;
	ImageFormat m_ImageFormat : 8;
	unsigned char m_Flags;

	union
	{
		unsigned int m_Texture; // One copy.
		unsigned int *m_Textures; // More than one copies.
#ifdef SHADERAPIOES3
		unsigned int m_DepthStencilSurface[1];
#else
		unsigned int m_DepthStencilSurface[2]; // One {depth, stencil} surface (or combo in depth where supported).
#endif
	};

	void Defaults(void)
	{
		m_UTexWrap = m_VTexWrap = 0x812F; // CLAMP_TO_EDGE
		m_CurrentCopy = 0;
		m_SwitchNeeded = 0;
		m_AnisotropicFilter = 0;
		m_pTextureGroupCounterFrame = NULL;
		m_pTextureGroupCounterGlobal = NULL;
		m_LastBoundFrame = -1;
		m_TimesBoundMax = 0;
		m_TimesBoundThisFrame = 0;
		m_Flags = 0;
	}

	unsigned int GetCurrentCopy(void)
	{
		Assert(!(m_Flags & IS_DEPTH_STENCIL));
		if (m_NumCopies == 1)
			return m_Texture;
		return m_Textures[m_CurrentCopy];
	}
};

class CPixelWriter;
class IMeshOES2;

class IShaderAPIOES2 : public IShaderAPI, public IShaderDevice, public IDebugTextureInfo
{
public:
	virtual void ApplyCullEnable(bool bEnable) = 0;
	virtual void ApplyFogMode(ShaderFogMode_t fogMode) = 0; // D0GHDR: Fog gamma correction.
	virtual void ApplyZBias(const ShadowState_t &state) = 0;
	virtual void ApplyZWriteEnable(bool bEnable) = 0;
	virtual void BindShaderProgram(ShaderProgramHandle_t program) = 0;
	virtual ShaderProgramHandle_t CreateShaderProgram(const char *pName, int combo, int format, void *pData) = 0;
	virtual void DrawMesh(IMeshOES2 *pMesh) = 0;
	virtual ShaderProgramHandle_t GetBoundProgramHandle(void) const = 0;
	virtual int GetCurrentFrameCounter(void) const = 0;
	virtual unsigned int GetFrontFace(void) const = 0;
	virtual ShaderProgramHandle_t GetShaderProgram(const char *pName, int combo) = 0;
	virtual const void *GetShaderProgramData(ShaderProgramHandle_t hProgram, int format) const = 0;
	virtual bool IsDeactivated(void) const = 0;
	virtual bool IsInSelectionMode(void) const = 0;
	virtual bool IsRenderingMesh(void) const = 0;
	virtual void RegisterSelectionHit(float minz, float maxz) = 0;
};

class CShaderAPIOES2 : public IShaderAPIOES2
{
public:
	CShaderAPIOES2(void);
	virtual ~CShaderAPIOES2(void);

	//-------------------------------------------------------------------------
	// ShaderDynamicAPI.
	//-------------------------------------------------------------------------

	// For information about the issue with this, read the DX8 comment.
	virtual double CurrentTime(void) const { return Plat_FloatTime(); }

	virtual void GetLightmapDimensions(int *w, int *h) { return ShaderUtil()->GetLightmapDimensions(w, h); }

	virtual MaterialFogMode_t GetSceneFogMode(void) { return m_SceneFogMode; }
	virtual void GetSceneFogColor(unsigned char *rgb)
		{ rgb[0] = m_SceneFogColor[0]; rgb[1] = m_SceneFogColor[1]; rgb[2] = m_SceneFogColor[2]; }

	virtual void MatrixMode(MaterialMatrixMode_t matrixMode)
		{ Assert((matrixMode >= 0) && (matrixMode < NUM_MATRIX_MODES)); m_CurrStack = matrixMode; }
	virtual void PushMatrix(void);
	virtual void PopMatrix(void);
	virtual void LoadMatrix(const VMatrix &m);
	virtual void MultMatrix(const VMatrix &m);
	virtual void MultMatrixLocal(const VMatrix &m);
	virtual void GetMatrix(MaterialMatrixMode_t matrixMode, VMatrix &dst) { dst = GetTransform(matrixMode); }
	virtual void LoadIdentity(void);
	virtual void LoadCameraToWorld(void);
	virtual void Ortho(double left, double right, double bottom, double top, double zNear, double zFar);
	virtual void PerspectiveX(double fovx, double aspect, double zNear, double zFar);
	virtual	void PickMatrix(int x, int y, int width, int height);
	virtual void Rotate(float angle, float x, float y, float z);
	virtual void Translate(float x, float y, float z);
	virtual void Scale(float x, float y, float z);
	virtual void ScaleXY(float x, float y);

	// FF.
	virtual void Color3f(float r, float g, float b) {}
	virtual void Color3fv(const float *pColor) {}
	virtual void Color4f(float r, float g, float b, float a) {}
	virtual void Color4fv(const float *pColor) {}
	virtual void Color3ub(unsigned char r, unsigned char g, unsigned char b) {}
	virtual void Color3ubv(const unsigned char *pColor) {}
	virtual void Color4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {}
	virtual void Color4ubv(const unsigned char *pColor) {}

	virtual void SetVertexShaderConstant(int var, const float *pVec, int numConst, bool bForce)
		{ AssertMsg(0, "CShaderAPIOES2::SetVertexShaderConstant: Use Uniform functions."); }
	virtual void SetPixelShaderConstant(int var, const float *pVec, int numConst, bool bForce)
		{ AssertMsg(0, "CShaderAPIOES2::SetPixelShaderConstant: Use Uniform functions."); }

	virtual void SetDefaultState(void) { MatrixMode(MATERIAL_MODEL); }

	virtual void GetWorldSpaceCameraPosition(float *pPos) const
	{
		pPos[0] = m_WorldSpaceCameraPosition[0];
		pPos[1] = m_WorldSpaceCameraPosition[1];
		pPos[2] = m_WorldSpaceCameraPosition[2];
	}

	virtual int GetCurrentNumBones(void) const { return m_DynamicState.m_NumBones; }

	virtual int GetCurrentLightCombo(void) const
		{ AssertMsg(0, "CShaderAPIOES2::GetCurrentLightCombo: DX8-only function."); return 0; }

	virtual MaterialFogMode_t GetCurrentFogType(void) const { return MATERIAL_FOG_NONE; } // FF.

	// Used only by DX8 multipass shaders.
	virtual void SetTextureTransformDimension(TextureStage_t textureStage, int dimension, bool projected) {}
	virtual void DisableTextureTransform(TextureStage_t textureStage) {}
	virtual void SetBumpEnvMatrix(TextureStage_t textureStage, float m00, float m01, float m10, float m11) {}

	virtual void SetVertexShaderIndex(int vshIndex)
		{ AssertMsg(0, "CShaderAPIOES2::SetVertexShaderIndex: Use transition table to bind shader programs."); }
	virtual void SetPixelShaderIndex(int pshIndex)
		{ AssertMsg(0, "CShaderAPIOES2::SetPixelShaderIndex: Use transition table to bind shader programs."); }

	virtual void GetBackBufferDimensions(int &width, int &height) const
	{
		if (m_DeviceActive)
		{
			width = m_BackBufferWidth;
			height = m_BackBufferHeight;
		}
		else
		{
			width = height = 0;
		}
	}

	virtual int GetMaxLights(void) const { return MAX_NUM_LIGHTS; }
	virtual const LightDesc_t &GetLight(int lightNum) const
	{
		Assert((lightNum >= 0) && (lightNum < MAX_NUM_LIGHTS));
		return m_DynamicState.m_LightDescs[lightNum];
	}

	// SetFogParams and SetStateAmbientLightCube set uniforms at the dynamic draw stage,
	// so they must be called in the same place as the Uniform functions.

	virtual void SetPixelShaderFogParams(int reg);

	virtual void SetVertexShaderStateAmbientLightCube(void);
	virtual void SetPixelShaderStateAmbientLightCube(int pshReg, bool bForceToBlack);
	virtual void CommitPixelShaderLighting(int pshReg);

	virtual CMeshBuilder *GetVertexModifyBuilder(void) { return &m_ModifyBuilder; }
	
	virtual bool InFlashlightMode(void) const { return ShaderUtil()->InFlashlightMode(); }
	virtual const FlashlightState_t &GetFlashlightState(VMatrix &worldToTexture) const
	{
		worldToTexture = m_FlashlightWorldToTexture;
		return m_FlashlightState;
	}

	virtual bool InEditorMode(void) const { return ShaderUtil()->InEditorMode(); }

	virtual MorphFormat_t GetBoundMorphFormat(void) { return ((MorphFormat_t)0); }

	virtual void BindStandardTexture(Sampler_t sampler, StandardTextureId_t id);

	virtual ITexture *GetRenderTargetEx(int nRenderTargetID) { return ShaderUtil()->GetRenderTargetEx(nRenderTargetID); }

	// D0GHDR
	virtual void SetToneMappingScaleLinear(const Vector &scale) {}
	virtual const Vector &GetToneMappingScaleLinear(void) const { return m_ToneMappingScale; }
	virtual float GetLightMapScaleFactor(void) const { return 4.5947933f; } // 2.0 ^ 2.2

	virtual void LoadBoneMatrix(int boneIndex, const float *m);

	virtual void PerspectiveOffCenterX(double fovx, double aspect, double zNear, double zFar,
		double bottom, double top, double left, double right);

	virtual void SetFloatRenderingParameter(int parm_number, float value);
	virtual void SetIntRenderingParameter(int parm_number, int value);
	virtual void SetVectorRenderingParameter(int parm_number, const Vector &value);
	virtual float GetFloatRenderingParameter(int parm_number) const;
	virtual int GetIntRenderingParameter(int parm_number) const;
	virtual Vector GetVectorRenderingParameter(int parm_number) const;

	virtual void SetStencilEnable(bool onoff);
	virtual void SetStencilFailOperation(StencilOperation_t op);
	virtual void SetStencilZFailOperation(StencilOperation_t op);
	virtual void SetStencilPassOperation(StencilOperation_t op);
	virtual void SetStencilCompareFunction(StencilComparisonFunction_t cmpfn);
	virtual void SetStencilReferenceValue(int ref);
	virtual void SetStencilTestMask(uint32 msk);
	virtual void SetStencilWriteMask(uint32 msk);
	virtual void ClearStencilBufferRectangle(int xmin, int ymin, int xmax, int ymax, int value);

	virtual void GetDXLevelDefaults(uint &max_dxlevel, uint &recommended_dxlevel)
		{ max_dxlevel = (recommended_dxlevel = 90); }

	virtual const FlashlightState_t &GetFlashlightStateEx(VMatrix &worldToTexture, ITexture **ppFlashlightDepthTexture) const
	{
		worldToTexture = m_FlashlightWorldToTexture;
		*ppFlashlightDepthTexture = m_pFlashlightDepthTexture;
		return m_FlashlightState;
	}

	virtual float GetAmbientLightCubeLuminance(void);

	virtual void GetDX9LightState(LightState_t *state) const;
	virtual int GetPixelFogCombo(void)
		{ return (m_SceneFogMode != MATERIAL_FOG_NONE) ? (m_SceneFogMode - 1) : MATERIAL_FOG_NONE; }

	virtual void BindStandardVertexTexture(VertexTextureSampler_t sampler, StandardTextureId_t id)
		{ AssertMsg(0, "CShaderAPIOES2::BindStandardVertexTexture: Use pixel texture functions for vertex textures."); }

	virtual bool IsHWMorphingEnabled(void) const { return false; }

	virtual void GetStandardTextureDimensions(int *pWidth, int *pHeight, StandardTextureId_t id)
		{ ShaderUtil()->GetStandardTextureDimensions(pWidth, pHeight, id); }

	virtual void SetBooleanVertexShaderConstant(int var, const BOOL *pVec, int numBools, bool bForce)
		{ AssertMsg(0, "CShaderAPIOES2::SetBooleanVertexShaderConstant: Use Uniform functions."); }
	virtual void SetIntegerVertexShaderConstant(int var, const int *pVec, int numIntVecs, bool bForce)
		{ AssertMsg(0, "CShaderAPIOES2::SetIntegerVertexShaderConstant: Use Uniform functions."); }
	virtual void SetBooleanPixelShaderConstant(int var, const BOOL *pVec, int numBools, bool bForce)
		{ AssertMsg(0, "CShaderAPIOES2::SetBooleanPixelShaderConstant: Use Uniform functions."); }
	virtual void SetIntegerPixelShaderConstant(int var, const int *pVec, int numIntVecs, bool bForce)
		{ AssertMsg(0, "CShaderAPIOES2::SetIntegerPixelShaderConstant: Use Uniform functions."); }

	virtual bool ShouldWriteDepthToDestAlpha(void) const { return false; } // Because of gl_FragCoord?

	virtual void PushDeformation(const DeformationBase_t *pDeformation)
		{ AssertMsg(0, "CShaderAPIOES2::PushDeformation: Deformations are not implemented."); }
	virtual void PopDeformation(void)
		{ AssertMsg(0, "CShaderAPIOES2::PopDeformation: Deformations are not implemented."); }
	virtual int GetNumActiveDeformations(void) const
		{ AssertMsg(0, "CShaderAPIOES2::GetNumActiveDeformations: Deformations are not implemented."); return 0; }
	virtual int GetPackedDeformationInformation(int nMaskOfUnderstoodDeformations, float *pConstantValuesOut,
		int nBufferSize, int nMaximumDeformations, int *pNumDefsOut) const
	{
		AssertMsg(0, "CShaderAPIOES2::GetPackedDeformationInformation: Deformations are not implemented.");
		*pNumDefsOut = 0;
		return 0;
	}

	// Using a different way of specifying vertex inputs.
	virtual void MarkUnusedVertexFields(unsigned int nFlags, int nTexCoordCount, bool *pUnusedTexCoords) {}

	virtual void ExecuteCommandBuffer(uint8 *pCmdBuffer)
		{ AssertMsg(0, "CShaderAPIOES2::ExecuteCommandBuffer: Command buffers are not supported."); }

	virtual void SetStandardTextureHandle(StandardTextureId_t nId, ShaderAPITextureHandle_t nHandle)
		{ Assert(nId < TEXTURE_MAX_STD_TEXTURES); m_StdTextureHandles[nId] = nHandle; }

	virtual void GetCurrentColorCorrection(ShaderColorCorrectionInfo_t *pInfo)
		{ ShaderUtil()->GetCurrentColorCorrection(pInfo); }

	virtual void SetDepthFeatheringPixelShaderConstant(int iConstant, float fDepthBlendScale)
		{ Uniform1f(iConstant, 192.0f / fDepthBlendScale); } // D0GHDR: Float HDR uses 8192, not 192.

	virtual void Uniform1f(int location, float v0);
	virtual void Uniform2f(int location, float v0, float v1);
	virtual void Uniform3f(int location, float v0, float v1, float v2);
	virtual void Uniform4f(int location, float v0, float v1, float v2, float v3);
	virtual void Uniform1i(int location, int v0);
	virtual void Uniform2i(int location, int v0, int v1);
	virtual void Uniform3i(int location, int v0, int v1, int v2);
	virtual void Uniform4i(int location, int v0, int v1, int v2, int v3);
	virtual void Uniform1fv(int location, unsigned int count, const float *value);
	virtual void Uniform2fv(int location, unsigned int count, const float *value);
	virtual void Uniform3fv(int location, unsigned int count, const float *value);
	virtual void Uniform4fv(int location, unsigned int count, const float *value);
	virtual void UniformMatrix2fv(int location, unsigned int count, const float *value);
	virtual void UniformMatrix3fv(int location, unsigned int count, const float *value);
	virtual void UniformMatrix4fv(int location, unsigned int count, const float *value);

	//-------------------------------------------------------------------------
	// ShaderAPI.
	//-------------------------------------------------------------------------

	virtual void SetViewports(int nCount, const ShaderViewport_t *pViewports);
	virtual int GetViewports(ShaderViewport_t *pViewports, int nMax) const;

	virtual void ClearBuffers(bool bClearColor, bool bClearDepth, bool bClearStencil,
		int renderTargetWidth, int renderTargetHeight);
	virtual void ClearColor3ub(unsigned char r, unsigned char g, unsigned char b);
	virtual void ClearColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

	// Asserts since in GL you bind programs, not shaders.
	virtual void BindVertexShader(VertexShaderHandle_t hVertexShader)
		{ AssertMsg(0, "CShaderAPIOES2::BindVertexShader: Use transition table to bind shader programs."); }
	virtual void BindGeometryShader(GeometryShaderHandle_t hGeometryShader)
		{ AssertMsg(0, "CShaderAPIOES2::BindGeometryShader: Use transition table to bind shader programs."); }
	virtual void BindPixelShader(PixelShaderHandle_t hPixelShader)
		{ AssertMsg(0, "CShaderAPIOES2::BindPixelShader: Use transition table to bind shader programs."); }

	virtual void SetRasterState(const ShaderRasterState_t &state) {} // Empty in DX8.

	virtual bool SetMode(void *hwnd, int nAdapter, const ShaderDeviceInfo_t &info);
	virtual void ChangeVideoMode(const ShaderDeviceInfo_t &info) {} // Native mode only.

	virtual StateSnapshot_t TakeSnapshot(void) { return m_TransitionTable.TakeSnapshot(); }

	virtual void TexMinFilter(ShaderTexFilterMode_t texFilterMode);
	virtual void TexMagFilter(ShaderTexFilterMode_t texFilterMode);
	virtual void TexWrap(ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode);

	virtual void CopyRenderTargetToTexture(ShaderAPITextureHandle_t hTexture)
		{ CopyRenderTargetToTextureEx(hTexture, 0, NULL, NULL); }

	virtual void Bind(IMaterial *pMaterial);

	virtual void FlushBufferedPrimitives(void);

	virtual IMesh *GetDynamicMesh(IMaterial *pMaterial, int nHWSkinBoneCount, bool bBuffered,
		IMesh *pVertexOverride, IMesh *pIndexOverride);
	virtual IMesh *GetDynamicMeshEx(IMaterial *pMaterial, VertexFormat_t vertexFormat, int nHWSkinBoneCount, bool bBuffered,
		IMesh *pVertexOverride, IMesh *pIndexOverride);

	virtual bool IsTranslucent(StateSnapshot_t id) const { return m_TransitionTable.GetSnapshot(id).m_AlphaBlendEnable; }
	virtual bool IsAlphaTested(StateSnapshot_t id) const { return m_TransitionTable.GetSnapshot(id).m_AlphaTestEnable; }
	virtual bool UsesVertexAndPixelShaders(StateSnapshot_t id) const { return true; }
	virtual bool IsDepthWriteEnabled(StateSnapshot_t id) const
	{
		const ShadowState_t &snapshot = m_TransitionTable.GetSnapshot(id);
		return snapshot.m_ZEnable && snapshot.m_ZWriteEnable;
	}

	virtual VertexFormat_t ComputeVertexFormat(int numSnapshots, StateSnapshot_t *pIds) const
		{ return ComputeVertexUsage(numSnapshots, pIds); }
	virtual VertexFormat_t ComputeVertexUsage(int numSnapshots, StateSnapshot_t *pIds) const;

	virtual void BeginPass(StateSnapshot_t snapshot);
	virtual void RenderPass(int nPass, int nPassCount);

	virtual void SetNumBoneWeights(int numBones);

	virtual void SetLight(int lightNum, const LightDesc_t &desc);
	virtual void SetLightingOrigin(Vector vLightingOrigin);
	virtual void SetAmbientLight(float r, float g, float b) {}
	virtual void SetAmbientLightCube(Vector4D cube[6]);

	virtual void ShadeMode(ShaderShadeMode_t mode) {}
	virtual void CullMode(MaterialCullMode_t cullMode);

	virtual void ForceDepthFuncEquals(bool bEnable)
		{ if (!IsDeactivated()) m_TransitionTable.ForceDepthFuncEquals(bEnable); }
	virtual void OverrideDepthEnable(bool bEnable, bool bDepthEnable)
		{ if (!IsDeactivated()) m_TransitionTable.OverrideDepthEnable(bEnable, bDepthEnable); }

	virtual void SetHeightClipZ(float z);
	virtual void SetHeightClipMode(MaterialHeightClipMode_t heightClipMode);

	virtual void SetClipPlane(int index, const float *pPlane)
		{ AssertMsg(0, "CShaderAPIOES2::SetClipPlane: User clip planes are not supported in OpenGL ES 2.0."); }
	virtual void EnableClipPlane(int index, bool bEnable)
		{ AssertMsg(0, "CShaderAPIOES2::EnableClipPlane: User clip planes are not supported in OpenGL ES 2.0."); }

	virtual void SetSkinningMatrices(void);

	virtual ImageFormat GetNearestSupportedFormat(ImageFormat fmt) const
		{ return OESFormatToImageFormat(ImageFormatToOESFormat(fmt)); }
	virtual ImageFormat GetNearestRenderTargetFormat(ImageFormat fmt) const
		{ AssertMsg(0, "CShaderAPIOES2::GetNearestRenderTargetFormat is unimplemented and unused."); return fmt; }

	virtual bool DoRenderTargetsNeedSeparateDepthBuffer(void) const { return true; } // Can't use FBO with default buffers.

	virtual ShaderAPITextureHandle_t CreateTexture(int width, int height, int depth,
		ImageFormat dstImageFormat, int numMipLevels, int numCopies, int creationFlags,
		const char *pDebugName, const char *pTextureGroupName);
	virtual void DeleteTexture(ShaderAPITextureHandle_t hTexture);
	virtual ShaderAPITextureHandle_t CreateDepthTexture(ImageFormat renderTargetFormat,
		int width, int height, const char *pDebugName, bool bTexture);
	virtual bool IsTexture(ShaderAPITextureHandle_t hTexture);
	virtual bool IsTextureResident(ShaderAPITextureHandle_t hTexture) { return true; }
	virtual void ModifyTexture(ShaderAPITextureHandle_t hTexture);
	virtual void TexImage2D(int level, int cubeFaceID, ImageFormat dstFormat, int zOffset,
		int width, int height, ImageFormat srcFormat, bool bSrcIsTiled, void *imageData);
	virtual void TexSubImage2D(int level, int cubeFaceID, int xOffset, int yOffset, int zOffset,
		int width, int height, ImageFormat srcFormat, int srcStride, bool bSrcIsTiled, void *imageData);
	virtual bool TexLock(int level, int cubeFaceID, int xOffset, int yOffset, int width, int height, CPixelWriter &writer);
	virtual void TexUnlock(void);
	virtual void TexSetPriority(int priority) {}
	virtual void BindTexture(Sampler_t sampler, ShaderAPITextureHandle_t hTexture);

	virtual void SetRenderTarget(ShaderAPITextureHandle_t colorTextureHandle, ShaderAPITextureHandle_t depthTextureHandle)
		{ SetRenderTargetEx(0, colorTextureHandle, depthTextureHandle); }

	virtual void ClearBuffersObeyStencil(bool bClearColor, bool bClearDepth);

	virtual void ReadPixels(int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat);
	virtual void ReadPixels(Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *data, ImageFormat dstFormat, int nDstStride);

	virtual void FlushHardware(void);

	virtual void BeginFrame(void) { ++m_CurrentFrame; m_TextureMemoryUsedLastFrame = 0; }
	virtual void EndFrame(void);

	virtual int SelectionMode(bool selectionMode);
	virtual void SelectionBuffer(unsigned int *pBuffer, int size);
	virtual void ClearSelectionNames(void);
	virtual void LoadSelectionName(int name);
	virtual void PushSelectionName(int name);
	virtual void PopSelectionName(void);

	virtual void ForceHardwareSync(void);

	virtual void ClearSnapshots(void);

	virtual void FogStart(float fStart);
	virtual void FogEnd(float fEnd);
	virtual void SetFogZ(float fogZ);
	virtual void SceneFogColor3ub(unsigned char r, unsigned char g, unsigned char b);
	// GetSceneFogColor is in ShaderDynamicAPI.
	virtual void SceneFogMode(MaterialFogMode_t fogMode);

	virtual bool CanDownloadTextures(void) const { return !IsDeactivated(); }

	virtual void ResetRenderState(bool bFullReset);

	virtual int GetCurrentDynamicVBSize(void) { return m_DynamicVBSize; }
	virtual void DestroyVertexBuffers(bool bExitingLevel);

	virtual void EvictManagedResources(void) {} // No managed resources in OES, also not performed on X360.

	virtual void SetAnisotropicLevel(int nAnisotropyLevel);

	virtual void SyncToken(const char *pToken) {} // No recording (yet?)

	// Always called with constant OVERBRIGHT.
	// LIGHT_INDEX is not used because OES2 doesn't use explicit uniform locations.
	// FLEXSCALE is not used because missing attributes are zeroed.
	// So, isn't really needed.
	virtual void SetStandardVertexShaderConstants(float fOverbright) {}

	virtual ShaderAPIOcclusionQuery_t CreateOcclusionQueryObject(void);
	virtual void DestroyOcclusionQueryObject(ShaderAPIOcclusionQuery_t hQuery);
	virtual void BeginOcclusionQueryDrawing(ShaderAPIOcclusionQuery_t hQuery);
	virtual void EndOcclusionQueryDrawing(ShaderAPIOcclusionQuery_t hQuery);
	virtual int OcclusionQuery_GetNumPixelsRendered(ShaderAPIOcclusionQuery_t hQuery, bool bFlush);

	virtual void SetFlashlightState(const FlashlightState_t &state, const VMatrix &worldToTexture)
		{ SetFlashlightStateEx(state, worldToTexture, NULL); }

	virtual void ClearVertexAndPixelShaderRefCounts(void);
	virtual void PurgeUnusedVertexAndPixelShaders(void);

	virtual void DXSupportLevelChanged(void) {} // Always simulating 90.

	virtual void EnableUserClipTransformOverride(bool bEnable);
	virtual void UserClipTransform(const VMatrix &worldToView);

	virtual MorphFormat_t ComputeMorphFormat(int numSnapshots, StateSnapshot_t *pIds) const { return 0; }

	virtual void SetRenderTargetEx(int nRenderTargetID,
		ShaderAPITextureHandle_t colorTextureHandle, ShaderAPITextureHandle_t depthTextureHandle);
	virtual void CopyRenderTargetToTextureEx(ShaderAPITextureHandle_t hTexture, int nRenderTargetID,
		Rect_t *pSrcRect, Rect_t *pDstRect);

	virtual void HandleDeviceLost(void) {} // Context lost is caused by window destruction on Android.

	virtual void EnableLinearColorSpaceFrameBuffer(bool bEnable) {} // D0GSRGB.

	virtual void SetFullScreenTextureHandle(ShaderAPITextureHandle_t h) {} // ReadPixels doesn't need it.

	// Rendering parameter functions are in ShaderDynamicAPI.

	virtual void SetFastClipPlane(const float *pPlane);
	virtual void EnableFastClip(bool bEnable);

	virtual void GetMaxToRender(IMesh *pMesh, bool bMaxUntilFlush, int *pMaxVerts, int *pMaxIndices);
	virtual int GetMaxVerticesToRender(IMaterial *pMaterial);
	virtual int GetMaxIndicesToRender(void) { return INDEX_BUFFER_SIZE; }

	// Stencil functions are in ShaderDynamicAPI.

	virtual void DisableAllLocalLights(void);

	virtual int CompareSnapshots(StateSnapshot_t snapshot0, StateSnapshot_t snapshot1);

	virtual IMesh *GetFlexMesh(void);

	virtual void SetFlashlightStateEx(const FlashlightState_t &state, const VMatrix &worldToTexture,
		ITexture *pFlashlightDepthTexture);

	virtual bool SupportsMSAAMode(int nMSAAMode) { return false; }

	virtual void EnableVSync_360(bool bEnable);

	virtual bool OwnGPUResources(bool bEnable) { return false; }

	virtual void GetFogDistances(float *fStart, float *fEnd, float *fFogZ);

	virtual void BeginPIXEvent(unsigned long color, const char *szName) {}
	virtual void EndPIXEvent(void) {}
	virtual void SetPIXMarker(unsigned long color, const char *szName) {}

	virtual void EnableAlphaToCoverage(void) {}
	virtual void DisableAlphaToCoverage(void) {}

	virtual void ComputeVertexDescription(unsigned char *pBuffer, VertexFormat_t vertexFormat, MeshDesc_t &desc) const;

	virtual bool SupportsShadowDepthTextures(void) { return HardwareConfig()->Caps().m_DepthTextureFormat != 0; }

	// Currently single-threaded (ST_SHADERAPI is defined in DX8 release).
	virtual void SetDisallowAccess(bool b) {}
	virtual void EnableShaderShaderMutex(bool b) {}
	virtual void ShaderLock(void) {}
	virtual void ShaderUnlock(void) {}

	virtual ImageFormat GetShadowDepthTextureFormat(void)
		{ return HardwareConfig()->Caps().m_DepthTextureFormat ? IMAGE_FORMAT_NV_DST24 : IMAGE_FORMAT_UNKNOWN; }

	virtual bool SupportsFetch4(void) { return false; } // ARB_texture_gather is not available in OES.

	virtual void SetShadowDepthBiasFactors(float fShadowSlopeScaleDepthBias, float fShadowDepthBias)
		{ m_ShadowSlopeScaleDepthBias = fShadowSlopeScaleDepthBias; m_ShadowDepthBias = fShadowDepthBias; }

	// New vertex/index buffer interface seems to be unused - NEWMESH is not defined as of The Orange Box.
	virtual void BindVertexBuffer(int streamID, IVertexBuffer *pVertexBuffer, int nOffsetInBytes,
		int nFirstVertex, int nVertexCount, VertexFormat_t fmt, int nRepetitions) { Assert(0); }
	virtual void BindIndexBuffer(IIndexBuffer *pIndexBuffer, int nOffsetInBytes) { Assert(0); }
	virtual void Draw(MaterialPrimitiveType_t primitiveType, int nFirstIndex, int nIndexCount) { Assert(0); }

	virtual void PerformFullScreenStencilOperation(void);

	virtual void SetScissorRect(const int nLeft, const int nTop, const int nRight, const int nBottom,
		const bool bEnableScissor);

	virtual bool SupportsCSAAMode(int nNumSamples, int nQualityLevel) { return false; }

	virtual void InvalidateDelayedShaderConstants(void) {} // Pixel fog is set per-shader in a different way.

	virtual float GammaToLinear_HardwareSpecific(float fGamma) const { return SrgbGammaToLinear(fGamma); }
	virtual float LinearToGamma_HardwareSpecific(float fLinear) const { return SrgbLinearToGamma(fLinear); }

	virtual void SetLinearToGammaConversionTextures(
		ShaderAPITextureHandle_t hSRGBWriteEnabledTexture, ShaderAPITextureHandle_t hIdentityTexture) {} // D0GSRGB.

	virtual ImageFormat GetNullTextureFormat(void) { return IMAGE_FORMAT_RGBA8888; }

	virtual void BindVertexTexture(VertexTextureSampler_t nSampler, ShaderAPITextureHandle_t hTexture)
		{ AssertMsg(0, "CShaderAPIOES2::BindVertexTexture: Use pixel texture functions for vertex textures."); }

	virtual void EnableHWMorphing(bool bEnable) {}
	virtual void SetFlexWeights(int nFirstWeight, int nCount, const MorphWeight_t *pWeights) {}

	virtual void FogMaxDensity(float flMaxDensity);

	virtual void CreateTextures(ShaderAPITextureHandle_t *pHandles, int count, int width, int height, int depth,
		ImageFormat dstImageFormat, int numMipLevels, int numCopies, int creationFlags,
		const char *pDebugName, const char *pTextureGroupName);

	virtual void AcquireThreadOwnership(void) {}
	virtual void ReleaseThreadOwnership(void) {}

	virtual bool SupportsNormalMapCompression(void) const { return false; }

	virtual void EnableBuffer2FramesAhead(bool bEnable) {}

	// SetDepthFeatheringPixelShaderConstant is in ShaderDynamicAPI.

	//-------------------------------------------------------------------------
	// ShaderDevice.
	//-------------------------------------------------------------------------

	virtual void ReleaseResources(void);
	virtual void ReacquireResources(void) { ReacquireResourcesInternal(); }

	virtual ImageFormat GetBackBufferFormat(void) const { return IMAGE_FORMAT_RGBA8888; }
	// GetBackBufferDimensions is in ShaderDynamicAPI.

	virtual int GetCurrentAdapter(void) const { return m_DeviceActive ? 0 : -1; }

	virtual bool IsUsingGraphics(void) const { return m_DeviceActive; }

	virtual void SpewDriverInfo(void) const;

	virtual int StencilBufferBits(void) const { return 8; }

	virtual bool IsAAEnabled(void) const { return false; } // Using FXAA instead.

	virtual void Present(void);

	virtual void GetWindowSize(int &nWidth, int &nHeight) const { GetBackBufferDimensions(nWidth, nHeight); }

	// No hardware gamma, assuming windowed mode, relying on Android settings.
	virtual void SetHardwareGammaRamp(float fGamma, float fGammaTVRangeMin, float fGammaTVRangeMax,
		float fGammaTVExponent, bool bTVEnabled) {}

	virtual bool AddView(void *hWnd) { return true; }
	virtual void RemoveView(void *hWnd) {}
	virtual void SetView(void *hWnd) {}

	// No point to do this in OES2.
	virtual IShaderBuffer *CompileShader(const char *pProgram, size_t nBufLen, const char *pShaderVersion)
		{ Assert(0); return NULL; }

	// Only snapshots can use shaders.
	virtual VertexShaderHandle_t CreateVertexShader(IShaderBuffer *pShaderBuffer)
		{ Assert(0); return VERTEX_SHADER_HANDLE_INVALID; }
	virtual void DestroyVertexShader(VertexShaderHandle_t hShader) { Assert(0); }
	virtual GeometryShaderHandle_t CreateGeometryShader(IShaderBuffer *pShaderBuffer)
		{ Assert(0); return GEOMETRY_SHADER_HANDLE_INVALID; }
	virtual void DestroyGeometryShader(GeometryShaderHandle_t hShader) { Assert(0); }
	virtual PixelShaderHandle_t CreatePixelShader(IShaderBuffer *pShaderBuffer)
		{ Assert(0); return PIXEL_SHADER_HANDLE_INVALID; }
	virtual void DestroyPixelShader(PixelShaderHandle_t hShader) { Assert(0); }

	virtual IMesh *CreateStaticMesh(VertexFormat_t vertexFormat, const char *pTextureBudgetGroup, IMaterial *pMaterial);
	virtual void DestroyStaticMesh(IMesh *pMesh);

	// NEWMESH stuff.
	virtual IVertexBuffer *CreateVertexBuffer(ShaderBufferType_t type, VertexFormat_t fmt, int nVertexCount,
		const char *pBudgetGroup) { Assert(0); return NULL; }
	virtual void DestroyVertexBuffer(IVertexBuffer *pVertexBuffer) { Assert(0); }
	virtual IIndexBuffer *CreateIndexBuffer(ShaderBufferType_t bufferType, MaterialIndexFormat_t fmt, int nIndexCount,
		const char *pBudgetGroup) { Assert(0); return NULL; }
	virtual void DestroyIndexBuffer(IIndexBuffer *pIndexBuffer) { Assert(0); }

	// NEWMESH stuff not even implemented in The Orange Box.
	virtual IVertexBuffer *GetDynamicVertexBuffer(int nStreamID, VertexFormat_t vertexFormat, bool bBuffered)
		{ Assert(0); return NULL; }
	virtual IIndexBuffer *GetDynamicIndexBuffer(MaterialIndexFormat_t fmt, bool bBuffered) { Assert(0); return NULL; }

	virtual void EnableNonInteractiveMode(MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t *pInfo) {} // X360.
	virtual void RefreshFrontBufferNonInteractive(void) {}

	//-------------------------------------------------------------------------
	// DebugTextureInfo.
	//-------------------------------------------------------------------------

	virtual void EnableDebugTextureList(bool bEnable) { m_EnableDebugTextureList = bEnable; }
	virtual void EnableGetAllTextures(bool bEnable) { m_DebugGetAllTextures = bEnable; }
	virtual KeyValues *GetDebugTextureList(void) { return m_pDebugTextureList; }
	virtual int GetTextureMemoryUsed(TextureMemoryType eTextureMemory);
	virtual bool IsDebugTextureListFresh(int numFramesAllowed)
	{
		return (m_DebugDataExportFrame <= m_CurrentFrame)
			&& (m_DebugDataExportFrame >= (m_CurrentFrame - numFramesAllowed));
	}
	virtual bool SetDebugTextureRendering(bool bEnable)
		{ bool bVal = m_DebugTexturesRendering; m_DebugTexturesRendering = bEnable; return bVal; }

	//-------------------------------------------------------------------------
	// ShaderAPIOES2.
	//-------------------------------------------------------------------------

	virtual void ApplyCullEnable(bool bEnable);
	virtual void ApplyFogMode(ShaderFogMode_t fogMode);
	virtual void ApplyZBias(const ShadowState_t &state);
	virtual void ApplyZWriteEnable(bool bEnable);
	virtual void BindShaderProgram(ShaderProgramHandle_t program);
	virtual ShaderProgramHandle_t CreateShaderProgram(const char *pName, int combo, int format, void *pData);
	virtual void DrawMesh(IMeshOES2 *pMesh);
	virtual ShaderProgramHandle_t GetBoundProgramHandle(void) const { return m_BoundProgram; }
	virtual int GetCurrentFrameCounter(void) const { return m_CurrentFrame; }
	virtual unsigned int GetFrontFace(void) const;
	virtual ShaderProgramHandle_t GetShaderProgram(const char *pName, int combo);
	virtual const void *GetShaderProgramData(ShaderProgramHandle_t hProgram, int format) const
		{ Assert(format == OES2_SHADER_FORMAT); return ProgramIsAllocated(hProgram) ? &(m_Programs[hProgram]) : NULL; }
	virtual bool IsDeactivated(void) const { return !m_DeviceActive || m_ReleaseResourcesRefCount; }
	virtual bool IsInSelectionMode(void) const { return m_InSelectionMode; }
	virtual bool IsRenderingMesh(void) const { return m_pRenderMesh != NULL; }
	virtual void RegisterSelectionHit(float minz, float maxz);

private:
	// Color format.
	unsigned int ImageFormatToOESFormat(ImageFormat fmt) const; // Returns PixelFormat|(PixelType<<16).
	ImageFormat OESFormatToImageFormat(unsigned int fmt) const; // Takes ImageFormatToOESFormat's return value.

	// Commit functions.
	void CommitFastClipPlane(void);
	void CommitSetScissorRect(void); // Call with m_ScissorRectChanged==true when the renderbuffer size changes.
	void CommitSetViewports(void);
	void CommitVertexShaderLighting(void);
	void CommitVertexShaderTransforms(void);

	// Device state.
	void InitDevice(void); // Initializes the context if the device needs reset.
	void ReacquireResourcesInternal(bool resetState = false,
		bool forceReacquire = false, const char *forceReason = NULL, bool resize = false);
	void ShutdownDevice(void);

	// Fast clipping.
	void UpdateFastClipUserClipPlane(void);

	// Fog.
	void UpdateFogConstant(void);

	// Light.
	VertexShaderLightTypes_t ComputeLightType(int i) const;
	void SortLights(int *index); // Depends on type and enabled/disabled.

	// Matrix stack.
	FORCEINLINE CUtlStack<VMatrix> &GetCurrentMatrixStack(void) { return m_MatrixStack[m_CurrStack]; }
	const VMatrix &GetProjectionMatrix(void)
		{ return m_DynamicState.m_FastClipEnabled ? m_CachedFastClipProjectionMatrix : GetTransform(MATERIAL_PROJECTION); }
	VMatrix &GetTransform(MaterialMatrixMode_t matrixMode) { return m_MatrixStack[matrixMode].Top(); }
	bool MatrixIsChanging(TransformType_t type = TRANSFORM_IS_GENERAL);
	FORCEINLINE void UpdateProjection(void)
		{ StandardConstantChanged(OES2_SHADER_CONST_MODELVIEWPROJ); StandardConstantChanged(OES2_SHADER_CONST_VIEWPROJ); }
	void UpdateMatrixTransform(TransformType_t type = TRANSFORM_IS_GENERAL);

	// Meshes.
	void FlushBufferedPrimitivesInternal(void);

	// Render state.
	void InitRenderState(void);

	// Render targets.
	void GetRenderTargetDimensions(int &width, int &height);

	// Shader programs.
	bool CommitStandardConstant(int index); // Returns true if need to reupload.
	unsigned int LoadOESShader(unsigned int type, const char * const *files, int nFiles, int version);
	FORCEINLINE ShaderProgram_t &GetBoundProgram(void) { return m_Programs[m_BoundProgram]; }
	FORCEINLINE int GetStandardConstLocation(int index) // Assuming IsProgramBound pre-check.
		{ return GetBoundProgram().m_StandardConstants[index].m_Location; }
	FORCEINLINE bool IsProgramBound(void) { return !IsDeactivated() && ProgramIsAllocated(m_BoundProgram); }
	FORCEINLINE bool ProgramIsAllocated(ShaderProgramHandle_t hProgram) const
		{ return m_Programs.IsValidIndex(hProgram) && m_Programs[hProgram].m_Program; }
	FORCEINLINE void StandardConstantChanged(int index)
	{
		if (m_StandardConstUpdateFrame[index] == m_CurrentFrame)
		{
			++m_StandardConstUpdate[index];
		}
		else
		{
			m_StandardConstUpdateFrame[index] = m_CurrentFrame;
			m_StandardConstUpdate[index] = 0;
		}
	}

	// Selection mode.
	void WriteHitRecord(void);

	// Stencil.
	void SetStencilOperation(unsigned int *target, StencilOperation_t op);

	// Textures.
	FORCEINLINE void AdvanceCurrentCopy(ShaderAPITextureHandle_t hTexture)
	{
		Texture_t &tex = m_Textures[hTexture];
		if (tex.m_NumCopies <= 1)
			return;
		if (++tex.m_CurrentCopy >= tex.m_NumCopies)
			tex.m_CurrentCopy = 0;
		UnbindTexture(hTexture);
	}
	FORCEINLINE Texture_t *BindModifyTexture(void)
	{
		if (m_ModifyTextureHandle == INVALID_SHADERAPI_TEXTURE_HANDLE)
			return NULL;
		BindTextureToTarget(m_ModifyTextureHandle);
		return &(m_Textures[m_ModifyTextureHandle]);
	}
	void BindTextureToTarget(ShaderAPITextureHandle_t hTexture);
	void CreateTextureHandles(ShaderAPITextureHandle_t *handles, int count);
	void DeleteOESTexture(Texture_t &tex);
	void SetActiveTexture(Sampler_t sampler);
	void SetUnpackAlignment(ImageFormat fmt, int width);
	void SetupTextureGroup(ShaderAPITextureHandle_t hTexture, const char *pTextureGroupName);
	void SetupTexParameters(const Texture_t &tex);
	void TexParameter(unsigned int pname, int param);
	FORCEINLINE bool TextureIsAllocated(ShaderAPITextureHandle_t hTexture)
		{ return m_Textures.IsValidIndex(hTexture) && (m_Textures[hTexture].m_Flags & Texture_t::IS_ALLOCATED); }
	void UnbindTexture(ShaderAPITextureHandle_t hTexture);
	bool WouldBeOverTextureLimit(ShaderAPITextureHandle_t hTexture);

	//-------------------------------------------------------------------------
	// Variables.
	//-------------------------------------------------------------------------

	// Camera.
	Vector m_WorldSpaceCameraPosition;

	// Clear.
	bool m_ClearColorChanged;
	bool m_ClearReverseDepth; // Cached value - was reverse depth enabled last frame.
	int m_ClearStencil;
	bool m_ZWriteEnable; // Remember the value when clearing.

	// Current frame.
	int m_CurrentFrame;

	// Depth bias.
	float m_ShadowSlopeScaleDepthBias;
	float m_ShadowDepthBias;

	int m_BackBufferWidth;
	int m_BackBufferHeight;

	// Device context.
	bool m_DeviceActive;
	void *m_EGLContext;
	void *m_EGLDisplay;
	void *m_EGLSurface;
	int m_ReleaseResourcesRefCount;
	bool m_WaitForVSync;

	// Dynamic state - the texture stage state that changes frequently.
	DynamicState_t m_DynamicState;

	// Fast clipping.
	VMatrix m_CachedFastClipProjectionMatrix;

	// Flashlight.
	FlashlightState_t m_FlashlightState;
	VMatrix m_FlashlightWorldToTexture;
	ITexture *m_pFlashlightDepthTexture;

	// Fog.
	unsigned char m_SceneFogColor[3];
	MaterialFogMode_t m_SceneFogMode;
	float m_FogConstant[4];

	// Framebuffers.
	CUtlFixedLinkedList<Framebuffer_t> m_Framebuffers;
	ShaderAPITextureHandle_t m_FramebufferColorTexture;
	ShaderAPITextureHandle_t m_FramebufferDepthTexture;

	// Light.
	Vector4D m_PixelShaderLighting[MAX_NUM_PIXEL_LIGHT_CONSTANTS]; // Cached CommitPixelShaderLighting lightState.
	struct
	{
		float m_Color[MAX_NUM_LIGHTS][4];
		float m_Direction[MAX_NUM_LIGHTS][4];
		float m_Position[MAX_NUM_LIGHTS][3];
		float m_Spot[MAX_NUM_LIGHTS][4];
		float m_Attenuation[MAX_NUM_LIGHTS][3];
	} m_VertexShaderLighting; // Cached CommitVertexShaderLighting lightState.
	bool m_PixelShaderLightingChanged; // Depends on SortLights deps, color, direction, origin and position.
	bool m_VertexShaderLightingChanged; // Depends on SortLights deps, color, direction, position, theta, phi, falloff and attenuation.

	// Matrix stack.
	MaterialMatrixMode_t m_CurrStack;
	CUtlStack<VMatrix> m_MatrixStack[NUM_MATRIX_MODES];

	// Mesh builder used to modify vertex data.
	CMeshBuilder m_ModifyBuilder;

	// Occlusion queries.
#ifdef _DEBUG
	ShaderAPIOcclusionQuery_t m_OcclusionQuery;
#endif

	// Render data.
	int m_DynamicVBSize;
	IMaterialInternal *m_pMaterial;
	IMeshOES2 *m_pRenderMesh;

	// Rendering parameter storage.
	float m_FloatRenderingParameters[MAX_FLOAT_RENDER_PARMS];
	int m_IntRenderingParameters[MAX_INT_RENDER_PARMS];
	Vector m_VectorRenderingParameters[MAX_VECTOR_RENDER_PARMS];

	// Scissor test pre-commit.
	bool m_ScissorEnabled;
	bool m_ScissorRectChanged;

	// Selection mode.
	unsigned int *m_pCurrSelectionRecord;
	bool m_InSelectionMode;
	int m_NumHits;
	unsigned int *m_pSelectionBuffer;
	unsigned int *m_pSelectionBufferEnd;
	float m_SelectionMaxZ;
	float m_SelectionMinZ;
	CUtlStack<int> m_SelectionNames;

	// Shader programs.
	ShaderProgramHandle_t m_BoundProgram;
	const int *m_BoundProgramConstants; // For faster access from dynamic state.
	CUtlDict<const char *, unsigned short> m_GlobalProgramSources; // Global source code files (prefixed with / when loaded).
	CUtlFixedLinkedList<ShaderProgram_t> m_Programs;

	int m_StandardConstUpdateFrame[OES2_SHADER_CONST_COUNT]; // The frame when the constant was last updated.
	int m_StandardConstUpdate[OES2_SHADER_CONST_COUNT]; // The number of updates this frame.

	// Shadow state transition table.
	CTransitionTable m_TransitionTable;
	StateSnapshot_t m_CurrentSnapshot;

	// Skinning.
	float m_BoneMatrix[16][12];

	// Textures.
	Sampler_t m_ActiveTexture;
	int m_AnisotropicLevel;
	ShaderAPITextureHandle_t m_BoundTextures[OES2_SHADER_MAX_SAMPLERS];
	ShaderAPITextureHandle_t m_ModifyTextureHandle;
	ShaderAPITextureHandle_t m_StdTextureHandles[TEXTURE_MAX_STD_TEXTURES];
	int m_TexLockLevel, m_TexLockCubeFaceID;
	int m_TexLockXOffset, m_TexLockYOffset;
	int m_TexLockWidth, m_TexLockHeight;
	unsigned char *m_pTexLockBits;
	CUtlFixedLinkedList<Texture_t> m_Textures;
	int m_UnpackAlignment;

	// Texture debug info.
	bool m_EnableDebugTextureList;
	bool m_DebugGetAllTextures;
	bool m_DebugTexturesRendering;
	KeyValues *m_pDebugTextureList;
	int m_TextureMemoryUsedLastFrame;
	int m_TextureMemoryUsedTotal;
	int m_TextureMemoryUsedPicMip1;
	int m_TextureMemoryUsedPicMip2;
	int m_DebugDataExportFrame;

	// Tone mapping.
	Vector m_ToneMappingScale; // D0GHDR: Change to 4D.

	// Viewport.
	ShaderViewport_t m_Viewport;
	bool m_ViewportChanged;
	bool m_ViewportZChanged;
};

extern CShaderAPIOES2 *g_pShaderAPIOES2;
FORCEINLINE IShaderAPIOES2 *ShaderAPI(void) { return g_pShaderAPIOES2; }

FORCEINLINE bool IsDeviceDeactivated(void) { return ShaderAPI()->IsDeactivated(); }

const char *OESErrorString(void);
const char *OESErrorString(unsigned int error);

#endif // !OES2_API_H