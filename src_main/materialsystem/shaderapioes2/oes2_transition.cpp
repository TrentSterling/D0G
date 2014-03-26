//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 transition table.
//
//===========================================================================//
#include <cstd/string.h>
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

CTransitionTable *g_pTransitionTable;

//-----------------------------------------------------------------------------
// Public interface.
//-----------------------------------------------------------------------------

int CShaderAPIOES2::CompareSnapshots(StateSnapshot_t snapshot0, StateSnapshot_t snapshot1)
{
	int dProgram = m_TransitionTable.GetSnapshotShader(snapshot0).m_ShaderProgram -
		m_TransitionTable.GetSnapshotShader(snapshot1).m_ShaderProgram;
	if (dProgram)
		return dProgram;
	return snapshot0 - snapshot1;
}

VertexFormat_t CShaderAPIOES2::ComputeVertexUsage(int numSnapshots, StateSnapshot_t *pIds) const
{
	if (numSnapshots <= 0)
		return 0;
	Assert(pIds);
	return m_TransitionTable.GetSnapshotShader(pIds[0]).m_VertexUsage;
}

CTransitionTable::CTransitionTable(void) :
	m_DefaultStateSnapshot(-1),
	m_CurrentShadowId(-1),
	m_CurrentSnapshotId(-1),
	m_TransitionOps(0, 8192),
	m_ShadowStateList(0, 256),
	m_TransitionTable(0, 256),
	m_SnapshotList(0, 256),
	m_ShadowStateDict(0, 256),
	m_SnapshotDict(0, 256),
	m_UniqueTransitions(0, 4096)
{
	Assert(!g_pTransitionTable);
	g_pTransitionTable = this;
}

CTransitionTable::~CTransitionTable(void)
{
	Assert(g_pTransitionTable == this);
	g_pTransitionTable = NULL;
}

void CTransitionTable::ForceDepthFuncEquals(bool bEnable)
{
	if (m_CurrentState.m_ForceDepthFuncEquals == bEnable)
		return;
	if (!(ShaderAPI()->IsRenderingMesh()))
		ShaderAPI()->FlushBufferedPrimitives();
	m_CurrentState.m_ForceDepthFuncEquals = bEnable;
	if (bEnable)
		SetZFunc(GL_EQUAL);
	else if (CurrentShadowState())
		SetZFunc(CurrentShadowState()->m_ZFunc);
}

void CTransitionTable::OverrideDepthEnable(bool bEnable, bool bDepthEnable)
{
	if (m_CurrentState.m_OverrideDepthEnable == bEnable)
		return;
	ShaderAPI()->FlushBufferedPrimitives();
	m_CurrentState.m_OverrideDepthEnable = bEnable;
	m_CurrentState.m_OverrideZWriteEnable = bDepthEnable;
	if (bEnable)
	{
		SetZEnable(true);
		ShaderAPI()->ApplyZWriteEnable(bDepthEnable);
	}
	else if (CurrentShadowState())
	{
		SetZEnable(CurrentShadowState()->m_ZEnable);
		ShaderAPI()->ApplyZWriteEnable(CurrentShadowState()->m_ZWriteEnable);
	}
}

void CTransitionTable::Reset(void)
{
	m_ShadowStateList.RemoveAll();
	m_SnapshotList.RemoveAll();
	m_TransitionTable.RemoveAll();
	m_TransitionOps.RemoveAll();
	m_ShadowStateDict.RemoveAll();
	m_SnapshotDict.RemoveAll();
	m_UniqueTransitions.RemoveAll();
	m_CurrentShadowId = -1;
	m_CurrentSnapshotId = -1;
	m_DefaultStateSnapshot = -1;
}

StateSnapshot_t CTransitionTable::TakeSnapshot(void)
{
	ShaderShadow()->ComputeAggregateShadowState();
	const ShadowState_t &currentState = ShaderShadow()->GetShadowState();

	ShadowStateId_t shadowStateId = FindShadowState(currentState);
	if (shadowStateId == -1)
	{
		shadowStateId = CreateShadowState(currentState);
		int i;
		for (i = 0; i < shadowStateId; ++i)
			CreateTransitionTableEntry(i, shadowStateId);
		for (i = 0; i < shadowStateId; ++i)
			CreateTransitionTableEntry(shadowStateId, i);
	}

	const ShadowShaderState_t &currentShaderState = ShaderShadow()->GetShadowShaderState();
	StateSnapshot_t snapshotId = FindStateSnapshot(shadowStateId, currentShaderState);
	if (snapshotId == -1)
		return CreateStateSnapshot(shadowStateId, currentShaderState);
	return snapshotId;
}

void CTransitionTable::UseDefaultState(void)
{
	if (IsDeviceDeactivated())
		return;
	VPROF("CTransitionTable::UseDefaultState");

	m_CurrentState.m_AlphaBlendEnable = false;
	m_CurrentState.m_SeparateAlphaBlendEnable = false;
	m_CurrentState.m_BlendFuncs.m_Src = GL_ONE;
	m_CurrentState.m_BlendFuncs.m_Dest = GL_ZERO;
	m_CurrentState.m_BlendFuncs.m_SrcAlpha = GL_ONE;
	m_CurrentState.m_BlendFuncs.m_DestAlpha = GL_ZERO;
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);

	m_CurrentState.m_ZEnable = true;
	m_CurrentState.m_ZFunc = GL_LEQUAL;
	m_CurrentState.m_ZBias = SHADER_POLYOFFSET_DISABLE;
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	m_CurrentState.m_OverrideDepthEnable = false;
	m_CurrentState.m_ForceDepthFuncEquals = false;
	ApplyTransition(m_DefaultTransition, m_DefaultStateSnapshot);

	m_CurrentSnapshotId = -1;
}

void CTransitionTable::UseSnapshot(StateSnapshot_t snapshotId)
{
	VPROF("CTransitionTable::UseSnapshot");
	SnapshotShaderState_t &snapshot = m_SnapshotList[snapshotId];
	ShadowStateId_t id = snapshot.m_ShadowStateId;
	if (m_CurrentSnapshotId == snapshotId)
		return;
	if (m_CurrentShadowId != id)
		ApplyTransition(m_TransitionTable[id][m_CurrentShadowId], id);
	// No need to set the program in the dynamic state. OES3 availability is known in the static state.
	ShaderAPI()->BindShaderProgram(snapshot.m_ShaderState.m_ShaderProgram);
	m_CurrentSnapshotId = snapshotId;
}

//-----------------------------------------------------------------------------
// Apply functions.
//-----------------------------------------------------------------------------

static void ApplyAlphaBlend(const ShadowState_t &state)
{
	g_pTransitionTable->ApplyAlphaBlend(state);
}

void CTransitionTable::ApplyAlphaBlend(const ShadowState_t &state)
{
	if (!state.m_AlphaBlendEnable && !state.m_SeparateAlphaBlendEnable)
	{
		if (m_CurrentState.m_AlphaBlendEnable || m_CurrentState.m_SeparateAlphaBlendEnable)
			glDisable(GL_BLEND);
	}
	else
	{
		if (!m_CurrentState.m_AlphaBlendEnable && !m_CurrentState.m_SeparateAlphaBlendEnable)
			glEnable(GL_BLEND);
		if (state.m_SeparateAlphaBlendEnable)
		{
			if (memcmp(&(m_CurrentState.m_BlendFuncs), &(state.m_BlendFuncs), sizeof(ShadowState_t::BlendFuncs_t)))
			{
				memcpy(&(m_CurrentState.m_BlendFuncs), &(state.m_BlendFuncs), sizeof(ShadowState_t::BlendFuncs_t));
				glBlendFuncSeparate(state.m_BlendFuncs.m_Src, state.m_BlendFuncs.m_Dest,
					state.m_BlendFuncs.m_SrcAlpha, state.m_BlendFuncs.m_DestAlpha);
			}
		}
		else
		{
			if ((m_CurrentState.m_BlendFuncs.m_Src != state.m_BlendFuncs.m_Src) ||
				(m_CurrentState.m_BlendFuncs.m_Dest != state.m_BlendFuncs.m_Dest))
			{
				m_CurrentState.m_BlendFuncs.m_Src = state.m_BlendFuncs.m_Src;
				m_CurrentState.m_BlendFuncs.m_Dest = state.m_BlendFuncs.m_Dest;
				glBlendFunc(state.m_BlendFuncs.m_Src, state.m_BlendFuncs.m_Dest);
			}
		}
	}
	m_CurrentState.m_AlphaBlendEnable = state.m_AlphaBlendEnable;
	m_CurrentState.m_SeparateAlphaBlendEnable = state.m_SeparateAlphaBlendEnable;
}

static void ApplyColorWriteEnable(const ShadowState_t &state)
{
	unsigned char mask = state.m_ColorWriteEnable;
	glColorMask(mask, mask, mask, state.m_AlphaWriteEnable);
}

static void ApplyCullEnable(const ShadowState_t &state)
{
	ShaderAPI()->ApplyCullEnable(state.m_CullEnable);
}

static void ApplyDepthTest(const ShadowState_t &state)
{
	g_pTransitionTable->ApplyDepthTest(state);
}

void CTransitionTable::ApplyDepthTest(const ShadowState_t &state)
{
	SetZEnable(state.m_ZEnable);
	if (!state.m_ZEnable)
		return;
	SetZFunc(state.m_ZFunc);
	if (m_CurrentState.m_ZBias != state.m_ZBias)
	{
		ShaderAPI()->ApplyZBias(state);
		m_CurrentState.m_ZBias = (PolygonOffsetMode_t)(state.m_ZBias); // Cast two bits from m_ZBias.
	}
}

static void ApplyFogMode(const ShadowState_t &state)
{
	ShaderAPI()->ApplyFogMode(state.m_FogMode); // D0GHDR: Fog gamma correction.
}

static void ApplyZWriteEnable(const ShadowState_t &state)
{
	ShaderAPI()->ApplyZWriteEnable(state.m_ZWriteEnable);
}

typedef void (*ApplyStateFunc_t)(const ShadowState_t &state);
ApplyStateFunc_t s_RenderFunctionTable[] = // Depends on RenderStateFunc_t!
{
	ApplyAlphaBlend,
	ApplyColorWriteEnable,
	ApplyCullEnable,
	ApplyDepthTest,
	ApplyFogMode,
	ApplyZWriteEnable
};

//-----------------------------------------------------------------------------
// Private functions.
//-----------------------------------------------------------------------------

void CTransitionTable::ApplyTransition(TransitionList_t &list, int snapshot)
{
	VPROF("CTransitionTable::ApplyTransition");
	if (IsDeviceDeactivated())
		return;

	int opCount = list.m_NumOperations;
	if (opCount > 0)
	{
		ShadowState_t &shadowState = m_ShadowStateList[snapshot];
		TransitionOp_t *pTransitionOp = &(m_TransitionOps[list.m_FirstOperation]);
		while (opCount--)
			(*s_RenderFunctionTable[*(pTransitionOp++)])(shadowState);
	}

	if (m_CurrentState.m_ForceDepthFuncEquals)
		SetZFunc(GL_EQUAL);
	if (m_CurrentState.m_OverrideDepthEnable)
	{
		SetZEnable(true);
		ShaderAPI()->ApplyZWriteEnable(m_CurrentState.m_OverrideZWriteEnable);
	}

	m_CurrentShadowId = snapshot;
}

CTransitionTable::ShadowStateId_t CTransitionTable::CreateShadowState(const ShadowState_t &currentState)
{
	int newShaderState = m_ShadowStateList.AddToTail();

	memcpy(&(m_ShadowStateList[newShaderState]), &currentState, sizeof(ShadowState_t));

	int i, newElem;
	TransitionList_t *newTransition;
	for (i = 0; i < newShaderState; ++i)
	{
		newElem = m_TransitionTable[i].AddToTail();
		newTransition = &(m_TransitionTable[i][newElem]);
		newTransition->m_FirstOperation = INVALID_TRANSITION_OP;
		newTransition->m_NumOperations = 0;
	}

	int newTransitionElem = m_TransitionTable.AddToTail();
	Assert(newShaderState == newTransitionElem);
	m_TransitionTable[newTransitionElem].EnsureCapacity(32);

	for (i = 0; i <= newShaderState; ++i)
	{
		newElem = m_TransitionTable[newShaderState].AddToTail();
		newTransition = &(m_TransitionTable[newShaderState][newElem]);
		newTransition->m_FirstOperation = INVALID_TRANSITION_OP;
		newTransition->m_NumOperations = 0;
	}

	ShadowStateDictEntry_t insert;
	CRC32_Init(&(insert.m_Checksum));
	CRC32_ProcessBuffer(&(insert.m_Checksum), &(m_ShadowStateList[newShaderState]), sizeof(ShadowState_t));
	CRC32_Final(&(insert.m_Checksum));
	insert.m_ShadowStateId = newShaderState;
	m_ShadowStateDict.Insert(insert);

	return newShaderState;
}

CTransitionTable::ShadowStateId_t CTransitionTable::FindShadowState(const ShadowState_t &currentState) const
{
	ShadowStateDictEntry_t find;

	CRC32_Init(&(find.m_Checksum));
	CRC32_ProcessBuffer(&(find.m_Checksum), &currentState, sizeof(ShadowState_t));
	CRC32_Final(&(find.m_Checksum));

	int dictCount = m_ShadowStateDict.Count();
	int i = m_ShadowStateDict.FindLessOrEqual(find);
	if (i < 0)
		return (ShadowStateId_t)(-1);

	const ShadowStateDictEntry_t *entry;
	ShadowStateId_t shadowState;
	for (; i < dictCount; ++i)
	{
		entry = &(m_ShadowStateDict[i]);
		if (entry->m_Checksum > find.m_Checksum)
			break;
		if (entry->m_Checksum != find.m_Checksum)
			continue;
		shadowState = entry->m_ShadowStateId;
		if (!memcmp(&(m_ShadowStateList[shadowState]), &currentState, sizeof(ShadowState_t)))
			return shadowState;
	}

	return (ShadowStateId_t)(-1);
}

StateSnapshot_t CTransitionTable::CreateStateSnapshot(ShadowStateId_t id, const ShadowShaderState_t &currentState)
{
	StateSnapshot_t snapshotId = m_SnapshotList.AddToTail();

	SnapshotShaderState_t &shaderState = m_SnapshotList[snapshotId];
	memcpy(&(shaderState.m_ShaderState), &currentState, sizeof(ShadowShaderState_t));
	shaderState.m_ShadowStateId = id;
	shaderState.m_ShaderState.m_Reserved = 0;
	shaderState.m_Reserved = 0; // Needed to get a good CRC.
	shaderState.m_Reserved2 = 0;

	SnapshotDictEntry_t insert;
	CRC32_Init(&(insert.m_Checksum));
	CRC32_ProcessBuffer(&(insert.m_Checksum), &shaderState, sizeof(SnapshotShaderState_t));
	CRC32_Final(&(insert.m_Checksum));
	insert.m_Snapshot = snapshotId;
	m_SnapshotDict.Insert(insert);

	return snapshotId;
}

StateSnapshot_t CTransitionTable::FindStateSnapshot(ShadowStateId_t id, const ShadowShaderState_t &currentState) const
{
	SnapshotShaderState_t temp;
	memcpy(&(temp.m_ShaderState), &currentState, sizeof(ShadowShaderState_t));
	temp.m_ShadowStateId = id;
	temp.m_ShaderState.m_Reserved = 0;
	temp.m_Reserved = 0; // Needed to get a good CRC.
	temp.m_Reserved2 = 0;

	SnapshotDictEntry_t find;

	CRC32_Init(&(find.m_Checksum));
	CRC32_ProcessBuffer(&(find.m_Checksum), &temp, sizeof(temp));
	CRC32_Final(&(find.m_Checksum));

	int dictCount = m_SnapshotDict.Count();
	int i = m_SnapshotDict.FindLessOrEqual(find);
	if (i < 0)
		return (StateSnapshot_t)(-1);
	StateSnapshot_t snapshot;
	for (; i < dictCount; ++i)
	{
		if (m_SnapshotDict[i].m_Checksum > find.m_Checksum)
			break;
		if (m_SnapshotDict[i].m_Checksum != find.m_Checksum)
			continue;
		snapshot = m_SnapshotDict[i].m_Snapshot;
		if ((id == m_SnapshotList[snapshot].m_ShadowStateId) &&
			!memcmp(&(m_SnapshotList[snapshot].m_ShaderState), &currentState, sizeof(ShadowShaderState_t)))
			return snapshot;
	}

	return (StateSnapshot_t)(-1);
}

void CTransitionTable::CreateTransitionTableEntry(int toIndex, int fromIndex)
{
	COMPILE_TIME_ASSERT(sizeof(s_RenderFunctionTable) == (sizeof(ApplyStateFunc_t) * RENDER_STATE_COUNT));

	int firstElem = m_TransitionOps.Count(), numOps;
	TransitionList_t *pTransition;
	int firstTest = INVALID_TRANSITION_OP;

	// Depends on RenderStateFunc_t!
	if (fromIndex < 0)
	{
		AddTransition(RENDER_STATE_AlphaBlend);
		AddTransition(RENDER_STATE_ColorWriteEnable);
		AddTransition(RENDER_STATE_CullEnable);
		AddTransition(RENDER_STATE_DepthTest);
		AddTransition(RENDER_STATE_FogMode);
		AddTransition(RENDER_STATE_ZWriteEnable);
		numOps = RENDER_STATE_COUNT;
		pTransition = &m_DefaultTransition;
		pTransition->m_NumOperations = RENDER_STATE_COUNT;
	}
	else
	{
		numOps = 0;
		const ShadowState_t &to = m_ShadowStateList[toIndex];
		const ShadowState_t &from = m_ShadowStateList[fromIndex];

		if ((to.m_AlphaBlendEnable != from.m_AlphaBlendEnable) ||
			(to.m_SeparateAlphaBlendEnable != from.m_SeparateAlphaBlendEnable) ||
			(to.m_AlphaBlendEnable && ((to.m_BlendFuncs.m_Src != from.m_BlendFuncs.m_Src) ||
				(to.m_BlendFuncs.m_Dest != from.m_BlendFuncs.m_Dest))) ||
			(to.m_SeparateAlphaBlendEnable && ((to.m_BlendFuncs.m_SrcAlpha != from.m_BlendFuncs.m_SrcAlpha) ||
				(to.m_BlendFuncs.m_DestAlpha != from.m_BlendFuncs.m_DestAlpha))))
			{ AddTransition(RENDER_STATE_AlphaBlend); ++numOps; }

		if ((to.m_ColorWriteEnable != from.m_ColorWriteEnable) || (to.m_AlphaWriteEnable != from.m_AlphaWriteEnable))
			{ AddTransition(RENDER_STATE_ColorWriteEnable); ++numOps; }

		if (to.m_CullEnable != from.m_CullEnable)
			{ AddTransition(RENDER_STATE_CullEnable); ++numOps; }

		if ((to.m_ZEnable != from.m_ZEnable) || (to.m_ZEnable &&
			((to.m_ZFunc != from.m_ZFunc) || (to.m_ZBias != from.m_ZBias))))
			{ AddTransition(RENDER_STATE_DepthTest); ++numOps; }

		if (to.m_FogMode != from.m_FogMode) // D0GHDR: Fog gamma correction.
			{ AddTransition(RENDER_STATE_FogMode); ++numOps; }

		if (to.m_ZWriteEnable != from.m_ZWriteEnable)
			{ AddTransition(RENDER_STATE_ZWriteEnable); ++numOps; }

		pTransition = &(m_TransitionTable[toIndex][fromIndex]);
		Assert(numOps <= 255);
		pTransition->m_NumOperations = numOps;
		if (!numOps)
		{
			pTransition->m_FirstOperation = INVALID_TRANSITION_OP;
			return;
		}

		TransitionList_t &diagonalList = m_TransitionTable[fromIndex][toIndex];
		if (diagonalList.m_NumOperations == numOps)
			firstTest = diagonalList.m_FirstOperation;
	}

	int identicalListFirstElem = FindIdenticalTransitionList(firstElem, numOps, firstTest);
	if (identicalListFirstElem != INVALID_TRANSITION_OP)
	{
		pTransition->m_FirstOperation = identicalListFirstElem;
		m_TransitionOps.RemoveMultiple(firstElem, numOps);
		return;
	}
	pTransition->m_FirstOperation = firstElem;
	m_UniqueTransitions.Insert(*pTransition);
	Assert((firstElem + numOps) < 65535);
	if ((firstElem + numOps) >= 65535)
		Warning("**** WARNING: Transition table overflow. Grab Brian or SiPlus\n");
}

unsigned short CTransitionTable::FindIdenticalTransitionList(unsigned short firstElem, unsigned short numOps,
	unsigned short firstTest) const
{
	VPROF("CTransitionTable::FindIdenticalTransitionList");
	const TransitionOp_t &op = m_TransitionOps[firstElem];
	if ((firstTest != INVALID_TRANSITION_OP) && !memcmp(&op, &(m_TransitionOps[firstTest]), numOps * sizeof(TransitionOp_t)))
		return firstTest;
	int i, count = m_UniqueTransitions.Count();
	for (i = 0; i < count; ++i)
	{
		const TransitionList_t &list = m_UniqueTransitions[i];
		if (list.m_NumOperations < numOps)
			return INVALID_TRANSITION_OP;
		int potentialMatch;
		int lastTest = list.m_FirstOperation + list.m_NumOperations - numOps;
		for (potentialMatch = list.m_FirstOperation; potentialMatch <= lastTest; ++potentialMatch)
		{
			if (m_TransitionOps[potentialMatch] == op)
				break;
		}
		if (potentialMatch > lastTest)
			continue;
		if (numOps == 1)
			return potentialMatch;
		if (!memcmp(&(m_TransitionOps[firstElem + 1]), &(m_TransitionOps[potentialMatch + 1]),
			(numOps - 1) * sizeof(TransitionOp_t)))
			return potentialMatch;
	}
	return INVALID_TRANSITION_OP;
}

void CTransitionTable::SetZEnable(bool enable)
{
	if (m_CurrentState.m_ZEnable == enable)
		return;
	if (enable)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	m_CurrentState.m_ZEnable = enable;
}

void CTransitionTable::SetZFunc(unsigned int cmpFunc)
{
	if (m_CurrentState.m_ZFunc == cmpFunc)
		return;
	glDepthFunc(cmpFunc);
	m_CurrentState.m_ZFunc = cmpFunc;
}