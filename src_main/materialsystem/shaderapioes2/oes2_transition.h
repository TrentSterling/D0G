//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 transition table interface.
//
//===========================================================================//
#ifndef OES2_TRANSITION_H
#define OES2_TRANSITION_H

#include "tier1/checksum_crc.h"
#include "tier1/utlsortvector.h"
#include "tier1/utlvector.h"

enum RenderStateFunc_t // s_RenderFunctionTable depends on this!
{
	RENDER_STATE_AlphaBlend, // SeparateAlphaBlend is a part of this.
	RENDER_STATE_ColorWriteEnable, // Also check AlphaWriteEnable.
	RENDER_STATE_CullEnable,
	RENDER_STATE_DepthTest,
	RENDER_STATE_FogMode, // D0GHDR: Fog gamma correction.
	RENDER_STATE_ZWriteEnable,
	RENDER_STATE_COUNT
};

class CTransitionTable
{
public:
	struct CurrentState_t
	{
		// Alpha state.
		bool m_AlphaBlendEnable;
		bool m_SeparateAlphaBlendEnable;
		ShadowState_t::BlendFuncs_t m_BlendFuncs;

		// Depth testing states.
		bool m_ZEnable;
		unsigned int m_ZFunc;
		PolygonOffsetMode_t m_ZBias;
		bool m_ForceDepthFuncEquals;
		bool m_OverrideDepthEnable;
		bool m_OverrideZWriteEnable;
	};

	CTransitionTable(void);
	virtual ~CTransitionTable(void);

	void Reset(void);

	StateSnapshot_t TakeSnapshot(void);

	FORCEINLINE void TakeDefaultStateSnapshot(void)
	{
		if (m_DefaultStateSnapshot == -1)
		{
			m_DefaultStateSnapshot = TakeSnapshot();
			CreateTransitionTableEntry(m_DefaultStateSnapshot, -1);
		}
	}

	void UseSnapshot(StateSnapshot_t snapshotId);

	void UseDefaultState(void);

	void ForceDepthFuncEquals(bool bEnable);
	void OverrideDepthEnable(bool bEnable, bool bDepthEnable);

	FORCEINLINE const ShadowState_t &GetSnapshot(StateSnapshot_t snapshotId) const
	{
		Assert((snapshotId >= 0) && (snapshotId < m_SnapshotList.Count()));
		return m_ShadowStateList[m_SnapshotList[snapshotId].m_ShadowStateId];
	}

	FORCEINLINE const ShadowShaderState_t &GetSnapshotShader(StateSnapshot_t snapshotId) const
	{
		Assert((snapshotId >= 0) && (snapshotId < m_SnapshotList.Count()));
		return m_SnapshotList[snapshotId].m_ShaderState;
	}

	FORCEINLINE const ShadowState_t *CurrentShadowState(void) const
	{
		if (m_CurrentShadowId == -1)
			return NULL;
		Assert((m_CurrentShadowId >= 0) && (m_CurrentShadowId < m_ShadowStateList.Count()));
		return &(m_ShadowStateList[m_CurrentShadowId]);
	}

	FORCEINLINE int CurrentSnapshot(void) const { return m_CurrentSnapshotId; }
	FORCEINLINE CurrentState_t &CurrentState(void) { return m_CurrentState; }

	void ApplyAlphaBlend(const ShadowState_t &state);
	void ApplyDepthTest(const ShadowState_t &state);

private:
	typedef short ShadowStateId_t;

	typedef unsigned char TransitionOp_t;

	enum { INVALID_TRANSITION_OP = 0xffff }; // Index, not opcode itself!

	struct ShadowStateDictEntry_t
	{
		CRC32_t m_Checksum;
		ShadowStateId_t m_ShadowStateId;
	};

	struct SnapshotDictEntry_t
	{
		CRC32_t m_Checksum;
		StateSnapshot_t m_Snapshot;
	};

	struct SnapshotShaderState_t
	{
		ShadowShaderState_t m_ShaderState;
		ShadowStateId_t m_ShadowStateId;
		unsigned short m_Reserved;	// Pad to 8 bytes.
		unsigned int m_Reserved2;
	};

	struct TransitionList_t
	{
		unsigned short m_FirstOperation;
		unsigned short m_NumOperations;
	};

	class ShadowStateDictLessFunc
	{
	public:
		bool Less(const ShadowStateDictEntry_t &src1, const ShadowStateDictEntry_t &src2, void *pCtx)
			{ return src1.m_Checksum < src2.m_Checksum; }
	};

	class SnapshotDictLessFunc
	{
	public:
		bool Less(const SnapshotDictEntry_t &src1, const SnapshotDictEntry_t &src2, void *pCtx)
			{ return src1.m_Checksum < src2.m_Checksum; }
	};

	class UniqueSnapshotLessFunc
	{
	public:
		bool Less(const TransitionList_t &src1, const TransitionList_t &src2, void *pCtx)
			{ return src1.m_NumOperations > src2.m_NumOperations; }
	};

	FORCEINLINE void AddTransition(RenderStateFunc_t func) { m_TransitionOps.AddToTail((TransitionOp_t)func); }

	void ApplyTransition(TransitionList_t &list, int snapshot);

	ShadowStateId_t CreateShadowState(const ShadowState_t &currentState);
	ShadowStateId_t FindShadowState(const ShadowState_t &currentState) const;

	StateSnapshot_t CreateStateSnapshot(ShadowStateId_t id, const ShadowShaderState_t &currentState);
	StateSnapshot_t FindStateSnapshot(ShadowStateId_t id, const ShadowShaderState_t &currentState) const;

	void CreateTransitionTableEntry(int toIndex, int fromIndex);

	unsigned short FindIdenticalTransitionList(unsigned short firstElem, unsigned short numOps, unsigned short firstTest) const;

	void SetZEnable(bool enable);
	void SetZFunc(unsigned int cmpFunc);

	// Variables.

	ShadowStateId_t m_CurrentShadowId;
	StateSnapshot_t m_CurrentSnapshotId;

	CurrentState_t m_CurrentState;

	StateSnapshot_t m_DefaultStateSnapshot;
	TransitionList_t m_DefaultTransition;

	CUtlSortVector<ShadowStateDictEntry_t, ShadowStateDictLessFunc> m_ShadowStateDict;
	CUtlVector<ShadowState_t> m_ShadowStateList;

	CUtlSortVector<SnapshotDictEntry_t, SnapshotDictLessFunc> m_SnapshotDict;
	CUtlVector<SnapshotShaderState_t> m_SnapshotList;

	CUtlVector<TransitionOp_t> m_TransitionOps;
	CUtlVector< CUtlVector<TransitionList_t> > m_TransitionTable;

	CUtlSortVector<TransitionList_t, UniqueSnapshotLessFunc> m_UniqueTransitions;
};

#endif // !OES2_TRANSITION_H