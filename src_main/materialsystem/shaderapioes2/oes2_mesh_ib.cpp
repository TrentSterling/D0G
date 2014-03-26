//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 mesh index buffer.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

unsigned short g_ScratchIndexBuffer[2];

CIndexBufferOES2::CIndexBufferOES2(ShaderBufferType_t type, MaterialIndexFormat_t fmt, int indexCount,
	const char *pBudgetGroupName)
{
	Assert(indexCount > 0);

	// See DX note why this is done.
	if (fmt == MATERIAL_INDEX_FORMAT_UNKNOWN)
	{
		fmt = MATERIAL_INDEX_FORMAT_16BIT;
		indexCount >>= 1;
	}

	m_BufferSize = indexCount * IndexSize();
	m_pBufferData = new unsigned char[m_BufferSize];
	m_FirstUnwrittenOffset = 0;
	m_Flush = false;
	m_IsDynamic = IsDynamicBufferType(type);
	m_IndexBuffer = 0;
	m_IndexCount = indexCount;
	m_IndexFormat = fmt;

#ifdef VPROF_ENABLED
	m_VProfFrame = -1;
#endif
}

CIndexBufferOES2::~CIndexBufferOES2(void)
{
	if (m_pBufferData)
	{
		delete[] m_pBufferData;
		m_pBufferData = NULL;
	}

	if (!m_IndexBuffer)
		return;

#ifdef VPROF_ENABLED
	if (m_IsDynamic)
	{
		VPROF_INCREMENT_GROUP_COUNTER("TexGroup_global_" TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER, 
			COUNTER_GROUP_TEXTURE_GLOBAL, -m_BufferSize);
	}
	else
	{
		VPROF_INCREMENT_GROUP_COUNTER("TexGroup_global_" TEXTURE_GROUP_STATIC_INDEX_BUFFER, 
			COUNTER_GROUP_TEXTURE_GLOBAL, -m_BufferSize);
	}
#endif

	MeshMgr()->DeleteOESBuffer(OES_BUFFER_TARGET_INDEX, m_IndexBuffer);
	m_IndexBuffer = 0;
}

bool CIndexBufferOES2::Allocate(void)
{
	Assert(!m_IndexBuffer);
	m_FirstUnwrittenOffset = 0;

	glGenBuffers(1, &m_IndexBuffer);
	if (!m_IndexBuffer)
	{
		Warning("CIndexBufferOES2::Allocate: glGenBuffers failed: %s.\n", OESErrorString());
		return false;
	}
	MeshMgr()->BindOESBuffer(OES_BUFFER_TARGET_INDEX, m_IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_BufferSize, NULL, m_IsDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

#ifdef VPROF_ENABLED
	if (m_IsDynamic)
	{
		VPROF_INCREMENT_GROUP_COUNTER("TexGroup_global_" TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER, 
			COUNTER_GROUP_TEXTURE_GLOBAL, m_BufferSize);
	}
	else
	{
		VPROF_INCREMENT_GROUP_COUNTER("TexGroup_global_" TEXTURE_GROUP_STATIC_INDEX_BUFFER, 
			COUNTER_GROUP_TEXTURE_GLOBAL, m_BufferSize);
	}
#endif

	return true;
}

void CIndexBufferOES2::CopyToTempMeshIndexBuffer(CMeshBuilder &dstMeshBuilder) const
{
	unsigned short *srcIndexArray = (unsigned short *)m_pBufferData;
	int i;
	for(i = 0; i < m_IndexCount; ++i)
	{
		dstMeshBuilder.Index(srcIndexArray[i]);
		dstMeshBuilder.AdvanceIndex();
	}
}

void CIndexBufferOES2::HandlePerFrameTextureStats(int frame)
{
#ifdef VPROF_ENABLED
	if ((m_VProfFrame != frame) && !m_IsDynamic)
	{
		m_VProfFrame = frame;
		VPROF_INCREMENT_GROUP_COUNTER("TexGroup_frame_" TEXTURE_GROUP_STATIC_INDEX_BUFFER, 
			COUNTER_GROUP_TEXTURE_PER_FRAME, m_BufferSize);
	}
#endif
}

bool CIndexBufferOES2::Lock(int nMaxIndexCount, bool bAppend, IndexDesc_t &desc)
{
	Assert(!m_pLockedData && (nMaxIndexCount != 0) && (nMaxIndexCount <= m_IndexCount));

	// DX FIXME: Why do we need to sync matrices now?
	ShaderUtil()->SyncMatrices();

	VPROF("CIndexBufferOES2::Lock");

	desc.m_nFirstIndex = 0;

	int memoryRequired;
	bool hasEnoughMemory;

	// IndexFormat cannot be MATERIAL_INDEX_FORMAT_UNKNOWN per the constructor, so not checking.
	if (!nMaxIndexCount || IsDeviceDeactivated())
		goto fail;

	if (nMaxIndexCount > m_IndexCount)
	{
		Warning("Too many indices for index buffer. . tell a programmer (%d>%d)\n", nMaxIndexCount, m_IndexCount);
		goto fail;
	}

	if (!m_IndexBuffer && !Allocate())
		goto fail;

	memoryRequired = nMaxIndexCount * IndexSize();
	hasEnoughMemory = ((m_FirstUnwrittenOffset + memoryRequired) <= m_BufferSize);

	if (bAppend)
	{
		Assert(!m_Flush);
		if (!hasEnoughMemory)
			goto fail;
	}
	else if (m_Flush || !hasEnoughMemory || !m_IsDynamic)
	{
		m_FirstUnwrittenOffset = 0;
		m_Flush = false;
	}

	desc.m_pIndices = (unsigned short *)(m_pBufferData + m_FirstUnwrittenOffset);
	desc.m_nIndexSize = IndexSize() >> 1;
	desc.m_nOffset = m_FirstUnwrittenOffset;
	return true;

fail:
	desc.m_pIndices = g_ScratchIndexBuffer;
	desc.m_nIndexSize = 0;
	desc.m_nOffset = 0;
	return false;
}

void CIndexBufferOES2::Spew(int nIndexCount, const IndexDesc_t &desc)
{
	Warning("\nIndices: %d (First %d, Offset %d)\n", nIndexCount, desc.m_nFirstIndex, desc.m_nOffset);
	SpewIndexBuffer(nIndexCount, desc);
}

void CIndexBufferOES2::SpewIndexBuffer(int indexCount, const IndexDesc_t &desc)
{
	char tempBuf[512];
	tempBuf[0] = '\0';
	int len = 0, i;
	char *pTemp = tempBuf;
	for (i = 0; i < indexCount; ++i)
	{
		len += Q_snprintf(pTemp, (sizeof(tempBuf) - 1) - len, "%d ", desc.m_pIndices[i]);
		if ((i & 15) != 15)
		{
			pTemp = tempBuf + len;
			continue;
		}
		Q_snprintf(pTemp, (sizeof(tempBuf) - 1) - len, "\n");
		Warning(tempBuf);
		tempBuf[0] = '\0';
		len = 0;
		pTemp = tempBuf;
	}
	Q_snprintf(pTemp, (sizeof(tempBuf) - 1) - len, "\n");
	Warning(tempBuf);
}

void CIndexBufferOES2::Unlock(int nWrittenIndexCount, IndexDesc_t &desc)
{
	if (!nWrittenIndexCount)
		return;
	Assert((m_FirstUnwrittenOffset + nWrittenIndexCount) <= m_IndexCount);
	MeshMgr()->BindOESBuffer(OES_BUFFER_TARGET_INDEX, m_IndexBuffer);
	int size = nWrittenIndexCount * IndexSize();
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_FirstUnwrittenOffset, size, m_pBufferData + m_FirstUnwrittenOffset);
	m_FirstUnwrittenOffset += size;
}