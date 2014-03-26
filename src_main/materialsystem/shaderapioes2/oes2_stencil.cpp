//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 stencil buffer.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Public interface.
//-----------------------------------------------------------------------------

void CShaderAPIOES2::SetStencilEnable(bool onoff)
{
	if (m_DynamicState.m_StencilEnabled == onoff)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_StencilEnabled = onoff;
	if (IsDeactivated())
		return;
	if (onoff)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
}

void CShaderAPIOES2::SetStencilFailOperation(StencilOperation_t op)
{
	SetStencilOperation(&(m_DynamicState.m_StencilFailOperation), op);
}

void CShaderAPIOES2::SetStencilZFailOperation(StencilOperation_t op)
{
	SetStencilOperation(&(m_DynamicState.m_StencilZFailOperation), op);
}

void CShaderAPIOES2::SetStencilPassOperation(StencilOperation_t op)
{
	SetStencilOperation(&(m_DynamicState.m_StencilPassOperation), op);
}

void CShaderAPIOES2::SetStencilCompareFunction(StencilComparisonFunction_t cmpfn)
{
	// Mapped to StencilComparisonFunction_t enum and StencilFunction GLenum!
	int fn = ((int)cmpfn) - (((int)STENCILCOMPARISONFUNCTION_NEVER) - GL_NEVER);
	if ((fn < GL_NEVER) || (fn > GL_ALWAYS))
	{
		Warning("CShaderAPIOES2::SetStencilCompareFunction: invalid cmpfn\n");
		return;
	}
	if (m_DynamicState.m_StencilCompareFunction == fn)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_StencilCompareFunction = fn;
	if (!IsDeactivated())
		glStencilFunc(fn, m_DynamicState.m_StencilReferenceValue, m_DynamicState.m_StencilTestMask);
}

void CShaderAPIOES2::SetStencilReferenceValue(int ref)
{
	if (m_DynamicState.m_StencilReferenceValue == ref)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_StencilReferenceValue = ref;
	if (!IsDeactivated())
		glStencilFunc(m_DynamicState.m_StencilCompareFunction, ref, m_DynamicState.m_StencilTestMask);
}

void CShaderAPIOES2::SetStencilTestMask(uint32 msk)
{
	if (m_DynamicState.m_StencilTestMask == msk)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_StencilTestMask = msk;
	if (!IsDeactivated())
		glStencilFunc(m_DynamicState.m_StencilCompareFunction, m_DynamicState.m_StencilReferenceValue, msk);
}

void CShaderAPIOES2::SetStencilWriteMask(uint32 msk)
{
	if (m_DynamicState.m_StencilWriteMask == msk)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_StencilWriteMask = msk;
	if (!IsDeactivated())
		glStencilMask(msk);
}

//-----------------------------------------------------------------------------
// Private API functions.
//-----------------------------------------------------------------------------

void CShaderAPIOES2::SetStencilOperation(unsigned int *target, StencilOperation_t op)
{
	// Mapped to StencilOperation_t enum!
	const unsigned int ops[] =
	{
		GL_KEEP,		// STENCILOPERATION_KEEP
		GL_ZERO,		// STENCILOPERATION_ZERO
		GL_REPLACE,		// STENCILOPERATION_REPLACE
		GL_INCR,		// STENCILOPERATION_INCRSAT
		GL_DECR,		// STENCILOPERATION_DECRSAT
		GL_INVERT,		// STENCILOPERATION_INVERT
		GL_INCR_WRAP,	// STENCILOPERATION_INCR
		GL_DECR_WRAP	// STENCILOPERATION_DECR
	};
	if ((op < STENCILOPERATION_KEEP) || (op > STENCILOPERATION_DECR))
	{
		Warning("CShaderAPIOES2::SetStencilOperation: invalid op\n");
		return;
	}
	unsigned int operation = ops[((int)op) - ((int)STENCILOPERATION_KEEP)];
	if (*target == operation)
		return;
	FlushBufferedPrimitives();
	*target = operation;
	if (!IsDeactivated())
	{
		glStencilOp(
			m_DynamicState.m_StencilFailOperation,
			m_DynamicState.m_StencilZFailOperation,
			m_DynamicState.m_StencilPassOperation);
	}
}