//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 occlusion queries.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
#include "oes2_glext.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

ShaderAPIOcclusionQuery_t CShaderAPIOES2::CreateOcclusionQueryObject(void)
{
#ifdef SHADERAPIOES3
	if (IsDeactivated())
#else
	if (!(HardwareConfig()->Caps().m_OcclusionQuery) || IsDeactivated())
#endif
		return INVALID_SHADERAPI_OCCLUSION_QUERY_HANDLE;
	unsigned int hQuery = INVALID_SHADERAPI_OCCLUSION_QUERY_HANDLE;
	glGenQueries(1, &hQuery);
	return hQuery;
}

void CShaderAPIOES2::DestroyOcclusionQueryObject(ShaderAPIOcclusionQuery_t hQuery)
{
	glDeleteQueries(1, &hQuery);
}

void CShaderAPIOES2::BeginOcclusionQueryDrawing(ShaderAPIOcclusionQuery_t hQuery)
{
	glBeginQuery(HardwareConfig()->Caps().m_OcclusionQuery, hQuery);
#ifdef _DEBUG
	m_OcclusionQuery = hQuery;
#endif
}

void CShaderAPIOES2::EndOcclusionQueryDrawing(ShaderAPIOcclusionQuery_t hQuery)
{
	Assert(m_OcclusionQuery == hQuery);
	glEndQuery(HardwareConfig()->Caps().m_OcclusionQuery);
#ifdef _DEBUG
	m_OcclusionQuery = INVALID_SHADERAPI_OCCLUSION_QUERY_HANDLE;
#endif
}

int CShaderAPIOES2::OcclusionQuery_GetNumPixelsRendered(ShaderAPIOcclusionQuery_t hQuery, bool bFlush)
{
	// Flush is implicit.
	unsigned int param;
	glGetQueryObjectuiv(hQuery, GL_QUERY_RESULT_AVAILABLE_EXT, &param);
	if (!param)
		return OCCLUSION_QUERY_RESULT_PENDING;
	glGetQueryObjectuiv(hQuery, GL_QUERY_RESULT_EXT, &param);
	return param;
}