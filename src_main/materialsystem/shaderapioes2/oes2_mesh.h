//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 mesh interface.
//
//===========================================================================//
#ifndef OES2_MESH_H
#define OES2_MESH_H

#include "materialsystem/imesh.h"
#include "tier1/utlmemory.h"
#include "tier1/utlvector.h"

//-----------------------------------------------------------------------------
// Vertex buffer.
//-----------------------------------------------------------------------------

abstract_class IVertexBufferOES2 : public IVertexBuffer
{
public:
	virtual void Flush(void) = 0;
	virtual unsigned int GetOESBuffer(void) const = 0;
	virtual void HandlePerFrameTextureStats(int frame) = 0;
	virtual void SetVertexFormat(VertexFormat_t fmt) = 0;
	virtual int VertexSize(void) const = 0;
};

class CVertexBufferOES2 : public IVertexBufferOES2
{
public:
	CVertexBufferOES2(ShaderBufferType_t type, VertexFormat_t fmt, int vertexCount, const char *pBudgetGroupName);
	virtual ~CVertexBufferOES2(void);

	virtual int VertexCount(void) const { return m_VertexCount; }
	virtual VertexFormat_t GetVertexFormat(void) const { return m_VertexFormat; }
	virtual bool IsDynamic(void) const { return m_IsDynamic; }
	virtual void BeginCastBuffer(VertexFormat_t format);
	virtual void EndCastBuffer(void);
	virtual int GetRoomRemaining(void) const { return (m_BufferSize - m_FirstUnwrittenOffset) / VertexSize(); }
	virtual bool Lock(int nMaxVertexCount, bool bAppend, VertexDesc_t &desc);
	virtual void Unlock(int nWrittenVertexCount, VertexDesc_t &desc);
	virtual void Spew(int nVertexCount, const VertexDesc_t &desc);
	virtual void ValidateData(int nVertexCount, const VertexDesc_t &desc);

	virtual void Flush(void) { m_Flush = m_IsDynamic; }
	virtual unsigned int GetOESBuffer(void) const { return m_VertexBuffer; }
	virtual void HandlePerFrameTextureStats(int frame);
	virtual void SetVertexFormat(VertexFormat_t fmt);
	virtual int VertexSize(void) const
		{ Assert(m_VertexFormat != VERTEX_FORMAT_UNKNOWN); return VertexFormatSize(m_VertexFormat);}

	static void ComputeVertexDescription(unsigned char *pBuffer, VertexFormat_t vertexFormat, VertexDesc_t &desc);
	static void PrintVertexFormat(VertexFormat_t fmt);
	static void SpewVertexBuffer(int vertexCount, const VertexDesc_t &desc);
	static void ValidateVertexBufferData(int vertexCount, const VertexDesc_t &desc, VertexFormat_t fmt);
	static int VertexFormatSize(VertexFormat_t fmt);

private:
	bool Allocate(void);

	int m_BufferSize;
	static unsigned char s_DynamicLockedData[DYNAMIC_VERTEX_BUFFER_MEMORY];
	int m_FirstUnwrittenOffset;
	bool m_Flush;
	bool m_IsDynamic;
	unsigned char *m_pLockedData; // Pointer to the dynamically allocated "locked" data.
	unsigned int m_VertexBuffer; // The GL buffer handle.
	int m_VertexCount;
	VertexFormat_t m_VertexFormat;

#ifdef VPROF_ENABLED
	int	*m_pFrameCounter;
	int	*m_pGlobalCounter;
	int m_VProfFrame;
#endif
};

//-----------------------------------------------------------------------------
// Index buffer.
//-----------------------------------------------------------------------------

abstract_class IIndexBufferOES2 : public IIndexBuffer
{
public:
	virtual void CopyToTempMeshIndexBuffer(CMeshBuilder &dstMeshBuilder) const = 0;
	virtual void Flush(void) = 0;
	virtual unsigned int GetOESBuffer(void) const = 0;
	virtual void HandlePerFrameTextureStats(int frame) = 0;
};

class CIndexBufferOES2 : public IIndexBufferOES2
{
public:
	CIndexBufferOES2(ShaderBufferType_t type, MaterialIndexFormat_t fmt, int indexCount, const char *pBudgetGroupName);
	virtual ~CIndexBufferOES2(void);

	virtual int IndexCount(void) const { Assert(!m_IsDynamic); return m_IndexCount; }
	virtual MaterialIndexFormat_t IndexFormat(void) const { Assert(!m_IsDynamic); return m_IndexFormat; }
	virtual bool IsDynamic(void) const { return m_IsDynamic; }
	// Can't recast index buffers in OES (and there's no point for), just like in DX.
	virtual void BeginCastBuffer(MaterialIndexFormat_t format)
	{
		Assert(format == MATERIAL_INDEX_FORMAT_16BIT); // No UNKNOWN, and OES2 doesn't support 32BIT (and it's unused).
		Assert(m_IsDynamic);
		Assert(!m_IndexFormat || (m_IndexFormat == format));
	}
	virtual void EndCastBuffer(void) {}
	virtual int GetRoomRemaining(void) const { return (m_BufferSize - m_FirstUnwrittenOffset) / IndexSize(); }
	virtual bool Lock(int nMaxIndexCount, bool bAppend, IndexDesc_t &desc);
	virtual void Unlock(int nWrittenIndexCount, IndexDesc_t &desc);
	virtual void ModifyBegin(bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t &desc) { Assert(0); }
	virtual void ModifyEnd(IndexDesc_t &desc) { Assert(0); }
	virtual void Spew(int nIndexCount, const IndexDesc_t &desc);
	virtual void ValidateData(int nIndexCount, const IndexDesc_t &desc) {} // Read DX note on why it's empty.

	virtual void CopyToTempMeshIndexBuffer(CMeshBuilder &dstMeshBuilder) const;
	virtual void Flush(void) { m_Flush = m_IsDynamic; }
	virtual unsigned int GetOESBuffer(void) const { return m_IndexBuffer; }
	virtual void HandlePerFrameTextureStats(int frame);

	static void SpewIndexBuffer(int indexCount, const IndexDesc_t &desc);

private:
	bool Allocate(void);
	FORCEINLINE int IndexSize(void) const { return 2; }

	unsigned char *m_pBufferData; // Copy in the client memory - no allocation in Lock, required for selection mode.
	int m_BufferSize;
	int m_FirstUnwrittenOffset;
	bool m_Flush;
	unsigned int m_IndexBuffer; // The GL buffer handle.
	int m_IndexCount;
	MaterialIndexFormat_t m_IndexFormat;
	bool m_IsDynamic;

#ifdef VPROF_ENABLED
	int m_VProfFrame;
#endif
};

extern unsigned short g_ScratchIndexBuffer[2];

//-----------------------------------------------------------------------------
// Base mesh.
//-----------------------------------------------------------------------------

abstract_class IMeshOES2 : public IMesh
{
public:
	virtual void BeginPass(void) = 0;
	virtual void RenderPass(void) = 0;
	virtual MaterialPrimitiveType_t GetPrimitiveType(void) const = 0;
	virtual IVertexBufferOES2 *GetVertexBuffer(void) const = 0;
	virtual IIndexBufferOES2 *GetIndexBuffer(void) const = 0;
	virtual bool HasColorMesh(void) const = 0;
	virtual bool HasEnoughRoom(int vertexCount, int indexCount) const = 0;
	virtual bool HasFlexMesh(void) const = 0;
	virtual bool NeedsVertexFormatReset(VertexFormat_t fmt) const = 0;
	virtual void PreLock(void) = 0;
	virtual void SetVertexFormat(VertexFormat_t fmt) = 0;
};

abstract_class CBaseMeshOES2 : public IMeshOES2
{
public:
	virtual ~CBaseMeshOES2(void) {}

	virtual int VertexCount(void) const { return 0; }

	virtual VertexFormat_t GetVertexFormat(void) const { return m_VertexFormat; }
	virtual void SetVertexFormat(VertexFormat_t fmt) { m_VertexFormat = fmt; }
	virtual MaterialIndexFormat_t IndexFormat(void) const { return MATERIAL_INDEX_FORMAT_16BIT; }

	virtual void BeginCastBuffer(VertexFormat_t format) { Assert(0); }
	virtual void BeginCastBuffer(MaterialIndexFormat_t format) { Assert(0); }
	virtual void EndCastBuffer(void) { Assert(0); }

	virtual int GetRoomRemaining(void) const { Assert(0); return 0; }

	// Optionally implemented methods of IVertexBuffer and IIndexBuffer.
	virtual bool Lock(int nVertexCount, bool bAppend, VertexDesc_t &desc) { Assert(0); return false; }
	virtual bool Lock(int nMaxIndexCount, bool bAppend, IndexDesc_t &desc) { Assert(0); return false; }

	virtual void Unlock(int nVertexCount, VertexDesc_t &desc) { Assert(0); }
	virtual void Unlock(int nWrittenIndexCount, IndexDesc_t &desc) { Assert(0); }

	// "CannotSupport" in CMatQueuedMesh, so no references - Lock is always used instead.
	virtual void ModifyBegin(bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t &desc) { Assert(0); }
	virtual void ModifyBegin(int nFirstVertex, int nVertexCount, int nFirstIndex, int nIndexCount,
		MeshDesc_t &desc) { Assert(0); }
	virtual void ModifyBeginEx(bool bReadOnly, int nFirstVertex, int nVertexCount, int nFirstIndex, int nIndexCount,
		MeshDesc_t &desc) { Assert(0); }
	virtual void ModifyEnd(IndexDesc_t &desc) { Assert(0); }
	virtual void ModifyEnd(MeshDesc_t &desc) { Assert(0); }

	virtual void SetColorMesh(IMesh *pColorMesh, int nVertexOffset) { Assert(0); }
	virtual bool HasColorMesh(void) const { return false; }

	virtual void Draw(CPrimList *pLists, int nLists) { AssertMsg(0, "CBaseMeshOES2::Draw: should never get here."); }

	virtual void CopyToMeshBuilder(int iStartVert, int nVerts,
		int iStartIndex, int nIndices, int indexOffset, CMeshBuilder &builder)
	{
		Warning("CopyToMeshBuilder called on something other than a temp mesh.\n");
		Assert(0);
	}

	virtual void Spew(int nVertexCount, const VertexDesc_t &desc);
	virtual void Spew(int nIndexCount, const IndexDesc_t &desc);
	virtual void Spew(int nVertexCount, int nIndexCount, const MeshDesc_t &desc);

	virtual void ValidateData(int nVertexCount, const VertexDesc_t &desc);
	virtual void ValidateData(int nIndexCount, const IndexDesc_t &desc) {}
	virtual void ValidateData(int nVertexCount, int nIndexCount, const MeshDesc_t &desc);

	virtual void SetFlexMesh(IMesh *pMesh, int nVertexOffset) { Assert(!pMesh && !nVertexOffset); }
	virtual void DisableFlexMesh(void) { Assert(0); }
	virtual bool HasFlexMesh(void) const { return false; }

	virtual void BeginPass(void) {}

	virtual IVertexBufferOES2 *GetVertexBuffer(void) const { return NULL; }
	virtual IIndexBufferOES2 *GetIndexBuffer(void) const { return NULL; }

	virtual bool HasEnoughRoom(int vertexCount, int indexCount) const { return true; }

	virtual bool NeedsVertexFormatReset(VertexFormat_t fmt) const { return m_VertexFormat != fmt; }

	virtual void PreLock(void) {}

	virtual void MarkAsDrawn(void) {}

protected:
	VertexFormat_t m_VertexFormat;

#ifdef VALIDATE_DEBUG
	IMaterialInternal *m_pMaterial;
#endif
};

//-----------------------------------------------------------------------------
// Static mesh.
//-----------------------------------------------------------------------------

class CMeshOES2 : public CBaseMeshOES2
{
public:
	CMeshOES2(const char *pTextureGroupName);
	virtual ~CMeshOES2(void);

	virtual int VertexCount(void) const { return m_pVertexBuffer ? m_pVertexBuffer->VertexCount() : 0; }
	virtual int IndexCount(void) const { return m_pIndexBuffer ? m_pIndexBuffer->IndexCount() : 0; }

	virtual bool IsDynamic(void) const { return false; }

	virtual bool Lock(int nVertexCount, bool bAppend, VertexDesc_t &desc);
	virtual bool Lock(int nMaxIndexCount, bool bAppend, IndexDesc_t &desc);
	virtual void LockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc);

	virtual void Unlock(int nWrittenVertexCount, VertexDesc_t &desc);
	virtual void Unlock(int nWrittenIndexCount, IndexDesc_t &desc);
	virtual void UnlockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc);

	virtual void SetPrimitiveType(MaterialPrimitiveType_t type);
	virtual MaterialPrimitiveType_t GetPrimitiveType(void) const { return m_Type; }

	virtual void Draw(int nFirstIndex, int nIndexCount);
	virtual void Draw(CPrimList *pLists, int nLists);

	virtual void SetColorMesh(IMesh *pColorMesh, int nVertexOffset);
	virtual bool HasColorMesh(void) const { return m_pColorMesh != NULL; }

	virtual void SetFlexMesh(IMesh *pMesh, int nVertexOffset);
	virtual void DisableFlexMesh(void) { SetFlexMesh(NULL, 0); }
	virtual bool HasFlexMesh(void) const { return m_pFlexVertexBuffer != NULL; }

	virtual IVertexBufferOES2 *GetVertexBuffer(void) const { return m_pVertexBuffer; }
	virtual IIndexBufferOES2 *GetIndexBuffer(void) const { return m_pIndexBuffer; }

	virtual void RenderPass(void);

protected:
	virtual void DrawInternal(const CPrimList *pLists, int nLists);

	virtual bool SetRenderState(int nVertexOffsetInBytes);

	virtual void UseVertexBuffer(IVertexBufferOES2 *pBuffer) { m_pVertexBuffer = pBuffer; }
	virtual void UseIndexBuffer(IIndexBufferOES2 *pBuffer) { m_pIndexBuffer = pBuffer; }

	IVertexBufferOES2 *m_pVertexBuffer;
	IIndexBufferOES2 *m_pIndexBuffer;

	IMeshOES2 *m_pColorMesh;
	int m_ColorMeshVertOffsetInBytes;

	IVertexBufferOES2 *m_pFlexVertexBuffer;
	int m_FlexVertOffsetInBytes;

	MaterialPrimitiveType_t m_Type;
	unsigned int m_Mode;

	int m_NumIndices;

	bool m_IsVBLocked;
	bool m_IsIBLocked;

	// Pass rendering stuff.
	static int s_nPrims;
	static const CPrimList *s_pPrims;

	const char *m_pTextureGroupName;
};

//-----------------------------------------------------------------------------
// Dynamic mesh.
//-----------------------------------------------------------------------------

class CDynamicMeshOES2 : public CMeshOES2
{
public:
	CDynamicMeshOES2(void) : CMeshOES2("CDynamicMeshOES2")
	{
		m_BufferId = 0;
		ResetVertexAndIndexCounts();
	}

	virtual int IndexCount(void) const { return m_TotalIndices; }
	virtual void SetVertexFormat(VertexFormat_t fmt);
	virtual bool IsDynamic(void) const { return true; }
	virtual void LockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc);
	virtual void UnlockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc);
	virtual void Draw(int nFirstIndex, int nIndexCount);
	virtual bool HasEnoughRoom(int vertexCount, int indexCount) const
	{
		return !IsDeviceDeactivated()
			&& (indexCount <= m_pIndexBuffer->GetRoomRemaining()) // Index is faster - division by 2, not VertexFormatSize.
			&& (vertexCount <= m_pVertexBuffer->GetRoomRemaining());
	}
	virtual bool NeedsVertexFormatReset(VertexFormat_t fmt) const
		{ return m_VertexOverride || m_IndexOverride || (m_VertexFormat != fmt); }
	virtual void PreLock(void) { if (m_HasDrawn) ResetVertexAndIndexCounts(); }
	virtual void MarkAsDrawn(void) { m_HasDrawn = true; }

private:
	FORCEINLINE void ResetVertexAndIndexCounts(void)
	{
		m_TotalVertices = m_TotalIndices = 0;
		m_FirstVertex = m_FirstIndex = -1;
		m_HasDrawn = false;
	}

	int m_BufferId;

	int m_TotalVertices;
	int m_TotalIndices;

	int m_FirstVertex;
	int m_FirstIndex;

	bool m_HasDrawn;

	bool m_VertexOverride;
	bool m_IndexOverride;

	// friend class CTempMeshOES2;
	// Used only in RenderPass, which is Assert(0).
	// void DrawSinglePassImmediately(void);

	friend class CMeshMgr;
	FORCEINLINE void Init(int bufferId) { m_BufferId = bufferId; }
	FORCEINLINE void OverrideVertexBuffer(IVertexBufferOES2 *pVertexBuffer)
		{ UseVertexBuffer(pVertexBuffer); m_VertexOverride = true; }
	FORCEINLINE void OverrideIndexBuffer(IIndexBufferOES2 *pIndexBuffer)
		{ UseIndexBuffer(pIndexBuffer); m_IndexOverride = true; }
	void Reset(void);
};

//-----------------------------------------------------------------------------
// Temporary mesh.
//-----------------------------------------------------------------------------

class CTempMeshOES2 : public CBaseMeshOES2
{
public:
	CTempMeshOES2(bool isDynamic);

	virtual int VertexCount(void) const { return m_VertexSize ? m_VertexData.Count() / m_VertexSize : 0; }
	virtual int IndexCount(void) const { return m_IndexData.Count(); }

	virtual void SetVertexFormat(VertexFormat_t fmt);

	virtual bool IsDynamic(void) const { return m_IsDynamic; }

	virtual void LockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc);
	virtual void UnlockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc);

	virtual void Draw(int nFirstIndex, int nIndexCount);

	virtual void SetPrimitiveType(MaterialPrimitiveType_t type) { Assert(type != MATERIAL_INSTANCED_QUADS); m_Type = type; }
	virtual MaterialPrimitiveType_t GetPrimitiveType(void) const { return m_Type; }

	virtual void CopyToMeshBuilder(int iStartVert, int nVerts,
		int iStartIndex, int nIndices, int indexOffset, CMeshBuilder &builder);

	virtual void BeginPass(void) { Assert(0); } // Don't try to render, ModifyVertexData is Assert(0).
	virtual void RenderPass(void) { Assert(0); }

private:
	void ClipTriangle(Vector **ppVert, float zNear, VMatrix &projection);
	void TestSelection(void);

	CUtlVector<unsigned short> m_IndexData;
	bool m_IsDynamic;
	int m_LockedIndices;
	int m_LockedVerts;
	MaterialPrimitiveType_t m_Type;
	CUtlVector< unsigned char, CUtlMemoryAligned<unsigned char, 32> > m_VertexData;
	int m_VertexSize;
};

//-----------------------------------------------------------------------------
// Buffered mesh.
//-----------------------------------------------------------------------------

class CBufferedMeshOES2 : public CBaseMeshOES2
{
public:
	CBufferedMeshOES2(void);

	// Inherited.
	virtual int IndexCount(void) const { Assert(0); return 0; }

	virtual VertexFormat_t GetVertexFormat(void) const { Assert(m_pMesh); return m_pMesh->GetVertexFormat(); }
	virtual void SetVertexFormat(VertexFormat_t fmt);
	virtual MaterialIndexFormat_t IndexFormat(void) const { Assert(0); return MATERIAL_INDEX_FORMAT_16BIT; }

	virtual bool IsDynamic(void) const { Assert(0); return true; }

	virtual void LockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc);

	virtual void UnlockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc);

	virtual void SetPrimitiveType(MaterialPrimitiveType_t type);
	virtual MaterialPrimitiveType_t GetPrimitiveType(void) const { return m_pMesh->GetPrimitiveType(); }

	virtual void Draw(int nFirstIndex, int nIndexCount);

	virtual void Spew(int nVertexCount, int nIndexCount, const MeshDesc_t &desc)
		{ if (m_pMesh) m_pMesh->Spew(nVertexCount, nIndexCount, desc); }

	virtual void ValidateData(int nVertexCount, int nIndexCount, const MeshDesc_t &desc)
	{
#ifdef VALIDATE_DEBUG
		Assert(m_pMesh);
		m_pMesh->ValidateData(nVertexCount, nIndexCount, desc);
#endif
	}

	virtual void SetFlexMesh(IMesh *pMesh, int nVertexOffset);

	virtual void RenderPass(void) { Assert(0); }

	// Custom.
	void Flush(void);

	FORCEINLINE const IMeshOES2 *GetMesh(void) const { return m_pMesh; }
	void SetMesh(IMeshOES2 *pMesh);

	FORCEINLINE bool WasRendered(void) const { return m_WasRendered; }

private:
	int m_ExtraIndices;
	bool m_FlushNeeded;
	bool m_IsFlushing;
	int m_LastIndex;
	IMeshOES2 *m_pMesh;
	bool m_WasRendered;
};

//-----------------------------------------------------------------------------
// Mesh manager.
//-----------------------------------------------------------------------------

// For BindOESBuffer - binding target.
enum OESBufferTarget_t
{
	OES_BUFFER_TARGET_VERTEX, // GL_ARRAY_BUFFER
	OES_BUFFER_TARGET_INDEX, // GL_ELEMENT_ARRAY_BUFFER
	OES_BUFFER_TARGET_COUNT
};

#define NUM_VERTEX_STREAMS 3

class CMeshMgr
{
public:
	CMeshMgr(void);

	void ApplyStreamState(void);
	void BindOESBuffer(OESBufferTarget_t target, unsigned int buffer);
	void DeleteOESBuffer(OESBufferTarget_t target, unsigned int buffer);
	void DestroyVertexBuffers(void);
	void DiscardVertexBuffers(void);
	IVertexBufferOES2 *FindOrCreateVertexBuffer(int dynamicBufferId, VertexFormat_t fmt);
	void Flush(void) { m_BufferedMesh.Flush(); }
	FORCEINLINE IMeshOES2 *GetActualDynamicMesh(VertexFormat_t fmt)
	{
		m_DynamicMesh.SetVertexFormat(fmt);
		return &m_DynamicMesh;
	}
	FORCEINLINE IIndexBufferOES2 *GetDynamicIndexBuffer(void) { return m_pDynamicIndexBuffer; }
	IMeshOES2 *GetDynamicMesh(IMaterial* pMaterial, VertexFormat_t fmt, int nHWSkinBoneCount, bool buffered,
		IMesh *pVertexOverride, IMesh *pIndexOverride);
	FORCEINLINE IMeshOES2 *GetFlexMesh(void)
	{
		m_DynamicFlexMesh.SetVertexFormat(VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_USE_EXACT_FORMAT);
		return &m_DynamicFlexMesh;
	}
	void GetMaxToRender(const IMesh *pMesh, bool maxUntilFlush, int *pMaxVerts, int *pMaxIndices);
	FORCEINLINE bool HasColorStream(void) const { return m_StreamState[1].m_Buffer != 0; }
	FORCEINLINE bool HasFlexStream(void) const { return m_StreamState[2].m_Buffer != 0; }
	FORCEINLINE bool IsDynamicMesh(const IMesh *pMesh) const { return (pMesh == &m_DynamicMesh) || (pMesh == &m_DynamicFlexMesh); }
	FORCEINLINE void MarkStreamStateDirty(void) { m_StreamStateProgram = SHADER_PROGRAM_HANDLE_INVALID; }
	void ReleaseBuffers(void);
	void RestoreBuffers(void);
	FORCEINLINE bool SetStreamState(unsigned int stream, unsigned int buffer, int offset, int stride)
	{
		StreamState_t &state = m_StreamState[stream];
		if ((state.m_Buffer == buffer) && (state.m_Offset == offset) && (state.m_Stride == stride))
			return false;
		state.m_Buffer = buffer;
		state.m_Offset = offset;
		state.m_Stride = stride;
		MarkStreamStateDirty();
		return true;
	}
	FORCEINLINE void SetVertexDecl(VertexFormat_t fmt)
	{
		if (m_VertexDeclFormat == fmt)
			return;
		m_VertexDeclFormat = fmt;
		CVertexBufferOES2::ComputeVertexDescription(NULL, fmt, m_VertexDecl);
		MarkStreamStateDirty();
	}
	FORCEINLINE void Shutdown(void) { CleanUp(); }

	// Should be private, but need access from global functions.
	void VertexAttribPointer(unsigned int stream, unsigned int index,
		int size, unsigned int type, const void *ptr, int normalized = false);
	void VertexAttribZero(unsigned int index);

	struct StreamState_t // Initialized with memset(0).
	{
		unsigned int m_Buffer;
		int m_Offset;
		int m_Stride;
	};

private:
	FORCEINLINE void CleanUp(void)
	{
		if (m_pDynamicIndexBuffer) { delete m_pDynamicIndexBuffer; m_pDynamicIndexBuffer = 0; }
		DestroyVertexBuffers();
		MarkStreamStateDirty();
	}
	void Init(void);

	unsigned int m_BoundBuffers[OES_BUFFER_TARGET_COUNT];

	CBufferedMeshOES2 m_BufferedMesh;
	bool m_BufferedMode;

	unsigned int m_CurrentAttributeArrays; // Attribute arrays used during the current ApplyStreamState call.
	unsigned int m_EnabledAttributeArrays;

	CDynamicMeshOES2 m_DynamicMesh;
	CDynamicMeshOES2 m_DynamicFlexMesh;
	CTempMeshOES2 m_DynamicTempMesh;

	IIndexBufferOES2 *m_pDynamicIndexBuffer;
	CUtlVector<IVertexBufferOES2 *> m_DynamicVertexBuffers;

	StreamState_t m_StreamState[NUM_VERTEX_STREAMS];
	ShaderProgramHandle_t m_StreamStateProgram; // Last program for which the stream state was applied.
	VertexDesc_t m_VertexDecl; // Stream 0 declaration.
	VertexFormat_t m_VertexDeclFormat; // Last format of stream 0.
};

extern CMeshMgr *g_pMeshMgr;
FORCEINLINE CMeshMgr *MeshMgr(void) { return g_pMeshMgr; }

#endif // !OES2_MESH_H