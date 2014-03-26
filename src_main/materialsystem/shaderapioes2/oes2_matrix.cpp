//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Matrix stack.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CShaderAPIOES2::CommitFastClipPlane(void)
{
	if (!m_DynamicState.m_FastClipPlaneChanged || !m_DynamicState.m_FastClipEnabled)
		return;
	m_DynamicState.m_FastClipPlaneChanged = false;

	m_CachedFastClipProjectionMatrix.Identity();

	VMatrix projection = GetTransform(MATERIAL_PROJECTION);
	// Pull in zNear because the shear in effect.
	// Moves it out: clipping artifacts when looking down at water. Could occur if this multiply is not done.
	projection[2][3] *= 0.5f;

	VMatrix worldToViewInvTrans =
		(m_DynamicState.m_UserClipTransformOverride ? m_DynamicState.m_UserClipTransform : GetTransform(MATERIAL_VIEW));
	VMatrix viewToProjectionInvTrans = projection;
	MatrixInverseGeneral(worldToViewInvTrans, worldToViewInvTrans);
	MatrixTranspose(worldToViewInvTrans, worldToViewInvTrans);
	MatrixInverseGeneral(viewToProjectionInvTrans, viewToProjectionInvTrans);
	MatrixTranspose(viewToProjectionInvTrans, viewToProjectionInvTrans);

	VPlane clipPlane = m_DynamicState.m_FastClipPlane;
	float radius = clipPlane.m_Normal.NormalizeInPlace();
	clipPlane.m_Dist = (radius != 0.0f) ? clipPlane.m_Dist / radius : 0.0f;

	worldToViewInvTrans.TransformPlane(clipPlane, clipPlane);
	viewToProjectionInvTrans.TransformPlane(clipPlane, clipPlane);

	// A plane with z * w > 0.4 at this point is behind the camera and will cause graphical glitches. Toss it.
	// 0.4 found through experimentation, but it was -0.4 in DX8 due to a different plane equation.
	if ((clipPlane.m_Normal.z * clipPlane.m_Dist) <= 0.4f)
	{
		Vector4D planeVector(clipPlane.m_Normal.x, clipPlane.m_Normal.y, clipPlane.m_Normal.z, -(clipPlane.m_Dist));
		Vector4DNormalize(planeVector);
		m_CachedFastClipProjectionMatrix[2][0] = planeVector.x;
		m_CachedFastClipProjectionMatrix[2][1] = planeVector.y;
		m_CachedFastClipProjectionMatrix[2][2] = planeVector.z;
		m_CachedFastClipProjectionMatrix[2][3] = planeVector.w;
	}

	VMatrix temp;
	MatrixMultiply(m_CachedFastClipProjectionMatrix, projection, temp);
	m_CachedFastClipProjectionMatrix = temp;
}

void CShaderAPIOES2::CommitVertexShaderTransforms(void)
{
	if (CommitStandardConstant(OES2_SHADER_CONST_MODELVIEWPROJ))
	{
		VMatrix temp, modelView;
		MatrixMultiply(GetTransform(MATERIAL_MODEL), GetTransform(MATERIAL_VIEW), temp);
		MatrixMultiply(temp, GetProjectionMatrix(), modelView);
		glUniformMatrix4fv(GetStandardConstLocation(OES2_SHADER_CONST_MODELVIEWPROJ), 1, GL_FALSE, modelView.Base());
	}
	if (CommitStandardConstant(OES2_SHADER_CONST_VIEWPROJ))
	{
		VMatrix view;
		MatrixMultiply(GetTransform(MATERIAL_VIEW), GetProjectionMatrix(), view);
		glUniformMatrix4fv(GetStandardConstLocation(OES2_SHADER_CONST_VIEWPROJ), 1, GL_FALSE, view.Base());
	}
	if (CommitStandardConstant(OES2_SHADER_CONST_MODEL))
	{
		glUniform4fv(GetStandardConstLocation(OES2_SHADER_CONST_MODEL), 3, GetTransform(MATERIAL_MODEL).Base());
	}
}

void CShaderAPIOES2::EnableFastClip(bool bEnable)
{
	if (m_DynamicState.m_FastClipEnabled == bEnable)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_FastClipEnabled = bEnable;
	UpdateProjection();
}

void CShaderAPIOES2::EnableUserClipTransformOverride(bool bEnable)
{
	if (m_DynamicState.m_UserClipTransformOverride == bEnable)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_UserClipTransformOverride = bEnable;
	m_DynamicState.m_FastClipPlaneChanged = true;
}

void CShaderAPIOES2::LoadCameraToWorld(void)
{
	if (!MatrixIsChanging(TRANSFORM_IS_CAMERA_TO_WORLD))
		return;
	VMatrix inv;
	MatrixInverseGeneral(GetTransform(MATERIAL_VIEW), inv); // Not using Transpose because it can be a scale matrix.
	inv.m[0][3] = inv.m[1][3] = inv.m[2][3] = 0.0f;
	GetCurrentMatrixStack().Top() = inv;
	UpdateMatrixTransform(TRANSFORM_IS_CAMERA_TO_WORLD);
}

void CShaderAPIOES2::LoadIdentity(void)
{
	if (!MatrixIsChanging(TRANSFORM_IS_IDENTITY))
		return;
	MatrixSetIdentity(GetCurrentMatrixStack().Top());
	UpdateMatrixTransform(TRANSFORM_IS_IDENTITY);
}

void CShaderAPIOES2::LoadMatrix(const VMatrix &m)
{
	if ((fabs(m[0][0] - 1.0f) < 0.001f) && (fabs(m[1][1] - 1.0f) < 0.001f) && (fabs(m[2][2] - 1.0f) < 0.001f) && (fabs(m[3][3] - 1.0f) < 0.001f) &&
		(fabs(m[0][1]) < 0.001f) && (fabs(m[0][2]) < 0.001f) && (fabs(m[0][3]) < 0.001f) &&
		(fabs(m[1][0]) < 0.001f) && (fabs(m[1][2]) < 0.001f) && (fabs(m[1][3]) < 0.001f) &&
		(fabs(m[2][0]) < 0.001f) && (fabs(m[2][1]) < 0.001f) && (fabs(m[2][3]) < 0.001f) &&
		(fabs(m[3][0]) < 0.001f) && (fabs(m[3][1]) < 0.001f) && (fabs(m[3][2]) < 0.001f))
	{
		LoadIdentity();
		return;
	}
	if (!MatrixIsChanging())
		return;
	GetCurrentMatrixStack().Top() = m;
	UpdateMatrixTransform();
}

bool CShaderAPIOES2::MatrixIsChanging(TransformType_t type)
{
	if (IsDeactivated())
		return false;
	if (GetCurrentMatrixStack().Count() <= 1)
		return false;
	if ((type != TRANSFORM_IS_GENERAL) && (type == m_DynamicState.m_TransformType[m_CurrStack]))
		return false;
	int textureMatrix = m_CurrStack - MATERIAL_TEXTURE0;
	if ((textureMatrix < 0) || (textureMatrix >= NUM_TEXTURE_TRANSFORMS))
		FlushBufferedPrimitivesInternal();
	return true;
}

void CShaderAPIOES2::MultMatrix(const VMatrix &m)
{
	if (!MatrixIsChanging())
		return;
	VMatrix &dst = GetCurrentMatrixStack().Top(), temp;
	MatrixMultiply(dst, m, temp);
	dst = temp;
	UpdateMatrixTransform();
}

void CShaderAPIOES2::MultMatrixLocal(const VMatrix &m)
{
	if (!MatrixIsChanging())
		return;
	VMatrix &dst = GetCurrentMatrixStack().Top(), temp;
	MatrixMultiply(m, dst, temp);
	dst = temp;
	UpdateMatrixTransform();
}

void CShaderAPIOES2::Ortho(double left, double top, double right, double bottom, double zNear, double zFar)
{
	if (!MatrixIsChanging())
		return;
	Assert(m_CurrStack == MATERIAL_PROJECTION);
	MatrixOrtho(GetCurrentMatrixStack().Top(), left, top, right, bottom, zNear, zFar);
	UpdateMatrixTransform();
}

void CShaderAPIOES2::PerspectiveOffCenterX(double fovx, double aspect, double zNear, double zFar,
	double bottom, double top, double left, double right)
{
	if (!MatrixIsChanging())
		return;
	Assert(m_CurrStack == MATERIAL_PROJECTION);
	MatrixPerspectiveOffCenterX(GetCurrentMatrixStack().Top(), fovx, aspect, zNear, zFar, bottom, top, left, right);
	UpdateMatrixTransform();
}

void CShaderAPIOES2::PerspectiveX(double fovx, double aspect, double zNear, double zFar)
{
	if (!MatrixIsChanging())
		return;
	Assert(m_CurrStack == MATERIAL_PROJECTION);
	MatrixPerspectiveX(GetCurrentMatrixStack().Top(), fovx, aspect, zNear, zFar);
	UpdateMatrixTransform();
}

void CShaderAPIOES2::PickMatrix(int x, int y, int width, int height)
{
	if (!MatrixIsChanging())
		return;
	Assert(m_CurrStack == MATERIAL_PROJECTION);

	ShaderViewport_t viewport;
	GetViewports(&viewport, 1);

	float vwidth = (float)(viewport.m_nWidth);
	float vheight = (float)(viewport.m_nHeight);
	float pw = 2.0f * (float)width / vwidth;
	float ph = 2.0f * (float)height / vheight;

	VMatrix matrix;
	matrix.Identity();
	matrix[0][0] = 2.0f / pw;
	matrix[1][1] = 2.0f / ph;
	matrix[0][3] = -2.0f / (2.0f * (float)(x - viewport.m_nTopLeftX) / vwidth - 1.0f) / pw;
	matrix[1][3] = -2.0f / (2.0f * (float)(y - viewport.m_nTopLeftY) / vheight - 1.0f) / ph;

	VMatrix &dst = GetCurrentMatrixStack().Top(), temp;
	MatrixMultiply(dst, matrix, temp);
	dst = temp;

	UpdateMatrixTransform();
}

void CShaderAPIOES2::PopMatrix(void)
{
	if (!MatrixIsChanging())
		return;
	GetCurrentMatrixStack().Pop();
	UpdateMatrixTransform();
}

void CShaderAPIOES2::PushMatrix(void)
{
	GetCurrentMatrixStack().Push();
}

void CShaderAPIOES2::Rotate(float angle, float x, float y, float z)
{
	if (!MatrixIsChanging())
		return;
	MatrixRotate(GetCurrentMatrixStack().Top(), Vector(x, y, z), angle);
	UpdateMatrixTransform();
}

void CShaderAPIOES2::Scale(float x, float y, float z)
{
	if (!MatrixIsChanging())
		return;
	VMatrix &dst = GetCurrentMatrixStack().Top(), trans, temp;
	MatrixBuildScale(trans, x, y, z);
	MatrixMultiply(dst, trans, temp);
	dst = temp;
	UpdateMatrixTransform();
}

void CShaderAPIOES2::ScaleXY(float x, float y)
{
	if (!MatrixIsChanging())
		return;
	VMatrix &dst = GetCurrentMatrixStack().Top(), trans, temp;
	MatrixBuildScale(trans, x, y, 1.0f);
	MatrixMultiply(dst, trans, temp);
	dst = temp;
	UpdateMatrixTransform();
}

void CShaderAPIOES2::SetFastClipPlane(const float *pPlane)
{
	VPlane &p = m_DynamicState.m_FastClipPlane;
	if ((p.m_Normal.x == pPlane[0]) && (p.m_Normal.y == pPlane[1]) && (p.m_Normal.z == pPlane[2]) && (p.m_Dist == pPlane[3]))
		return;
	FlushBufferedPrimitives();
	p.m_Normal.x = pPlane[0];
	p.m_Normal.y = pPlane[1];
	p.m_Normal.z = pPlane[2];
	p.m_Dist = pPlane[3];
	m_DynamicState.m_FastClipPlaneChanged = true;
	UpdateProjection();
}

void CShaderAPIOES2::SetHeightClipMode(MaterialHeightClipMode_t heightClipMode)
{
	if (m_DynamicState.m_HeightClipMode == heightClipMode)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_HeightClipMode = heightClipMode;
	UpdateFastClipUserClipPlane();
}

void CShaderAPIOES2::SetHeightClipZ(float z)
{
	if (m_DynamicState.m_HeightClipZ == z)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_HeightClipZ = z;
	UpdateFastClipUserClipPlane();
}

void CShaderAPIOES2::Translate(float x, float y, float z)
{
	if (!MatrixIsChanging())
		return;
	VMatrix &dst = GetCurrentMatrixStack().Top(), trans, temp;
	MatrixBuildTranslation(trans, x, y, z);
	MatrixMultiply(dst, trans, temp);
	dst = temp;
	UpdateMatrixTransform();
}

void CShaderAPIOES2::UpdateFastClipUserClipPlane(void)
{
	float plane[4];
	switch (m_DynamicState.m_HeightClipMode)
	{
	case MATERIAL_HEIGHTCLIPMODE_DISABLE:
		EnableFastClip(false);
		break;
	case MATERIAL_HEIGHTCLIPMODE_RENDER_ABOVE_HEIGHT:
		plane[0] = 0.0f;
		plane[1] = 0.0f;
		plane[2] = 1.0f;
		plane[3] = m_DynamicState.m_HeightClipZ;
		EnableFastClip(true);
		SetFastClipPlane(plane);
		break;
	case MATERIAL_HEIGHTCLIPMODE_RENDER_BELOW_HEIGHT:
		plane[0] = 0.0f;
		plane[1] = 0.0f;
		plane[2] = -1.0f;
		plane[3] = -(m_DynamicState.m_HeightClipZ);
		EnableFastClip(true);
		SetFastClipPlane(plane);
		break;
	}
	UpdateProjection();
}

void CShaderAPIOES2::UpdateMatrixTransform(TransformType_t type)
{
	m_DynamicState.m_TransformType[m_CurrStack] = type;
	if ((m_CurrStack == MATERIAL_VIEW) || (m_CurrStack == MATERIAL_PROJECTION) || (m_CurrStack == MATERIAL_MODEL))
	{
		StandardConstantChanged(OES2_SHADER_CONST_MODELVIEWPROJ);
		if (m_CurrStack == MATERIAL_MODEL)
		{
			if (!(m_DynamicState.m_NumBones))
				StandardConstantChanged(OES2_SHADER_CONST_MODEL);
		}
		else
		{
			StandardConstantChanged(OES2_SHADER_CONST_VIEWPROJ);
			if (m_CurrStack == MATERIAL_VIEW)
			{
				VMatrix &view = GetTransform(MATERIAL_VIEW);
				m_WorldSpaceCameraPosition[0] = -(
					view[0][3] * view[0][0] +
					view[1][3] * view[1][0] +
					view[2][3] * view[2][0]);
				m_WorldSpaceCameraPosition[1] = -(
					view[0][3] * view[0][1] +
					view[1][3] * view[1][1] +
					view[2][3] * view[2][1]);
				m_WorldSpaceCameraPosition[2] = -(
					view[0][3] * view[0][2] +
					view[1][3] * view[1][2] +
					view[2][3] * view[2][2]);
			}
			if ((m_CurrStack == MATERIAL_PROJECTION) ||
				((m_CurrStack == MATERIAL_VIEW) && !(m_DynamicState.m_UserClipTransformOverride)))
				m_DynamicState.m_FastClipPlaneChanged = true;
		}
	}
}

void CShaderAPIOES2::UserClipTransform(const VMatrix &worldToView)
{
	if (m_DynamicState.m_UserClipTransform == worldToView)
		return;
	m_DynamicState.m_UserClipTransform = worldToView;
	if (m_DynamicState.m_UserClipTransformOverride)
	{
		FlushBufferedPrimitives();
		m_DynamicState.m_FastClipPlaneChanged = true;
	}
}