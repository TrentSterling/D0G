//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: Selection mode.
//
//===========================================================================//
#include <float.h>
#include "oes2.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CShaderAPIOES2::ClearSelectionNames(void)
{
	if (m_InSelectionMode)
		WriteHitRecord();
	m_SelectionNames.Clear();
}

void CShaderAPIOES2::LoadSelectionName(int name)
{
	if (!m_InSelectionMode)
		return;
	WriteHitRecord();
	Assert(m_SelectionNames.Count() > 0);
	m_SelectionNames.Top() = name;
}

void CShaderAPIOES2::PopSelectionName(void)
{
	if (!m_InSelectionMode)
		return;
	WriteHitRecord();
	m_SelectionNames.Pop();
}

void CShaderAPIOES2::PushSelectionName(int name)
{
	if (!m_InSelectionMode)
		return;
	WriteHitRecord();
	m_SelectionNames.Push(name);
}

void CShaderAPIOES2::RegisterSelectionHit(float minz, float maxz)
{
	if (minz < 0.0f)
		minz = 0.0f;
	if (maxz > 1.0f);
		maxz = 1.0f;
	if (m_SelectionMinZ > minz)
		m_SelectionMinZ = minz;
	if (m_SelectionMaxZ < maxz)
		m_SelectionMaxZ = maxz;
}

void CShaderAPIOES2::SelectionBuffer(unsigned int *pBuffer, int size)
{
	Assert(!m_InSelectionMode);
	Assert(pBuffer && size);
	m_pSelectionBuffer = pBuffer;
	m_pSelectionBufferEnd = pBuffer + size;
	m_pCurrSelectionRecord = pBuffer;
}

int CShaderAPIOES2::SelectionMode(bool selectionMode)
{
	int numHits = m_NumHits;
	if (m_InSelectionMode)
		WriteHitRecord();
	m_InSelectionMode = selectionMode;
	m_pCurrSelectionRecord = m_pSelectionBuffer;
	m_NumHits = 0;
	return numHits;
}

void CShaderAPIOES2::WriteHitRecord(void)
{
	FlushBufferedPrimitives();
	if (m_SelectionNames.Count() && (m_SelectionMinZ != FLT_MAX))
	{
		Assert((m_pCurrSelectionRecord + m_SelectionNames.Count() + 3) < m_pSelectionBufferEnd);
		*(m_pCurrSelectionRecord++) = m_SelectionNames.Count();
	    *(m_pCurrSelectionRecord++) = (int)((double)m_SelectionMinZ * (double)0xffffffff);
	    *(m_pCurrSelectionRecord++) = (int)((double)m_SelectionMaxZ * (double)0xffffffff);
		int i;
		for (i = 0; i < m_SelectionNames.Count(); ++i)
			*(m_pCurrSelectionRecord++) = m_SelectionNames[i];
		++m_NumHits;
	}
	m_SelectionMinZ = FLT_MAX;
	m_SelectionMaxZ = FLT_MIN;
}