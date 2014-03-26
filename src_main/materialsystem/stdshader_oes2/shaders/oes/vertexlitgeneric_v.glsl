uniform vec4 c_BaseTexCoordTransform[2];
uniform mat4 s_ViewProj;
uniform vec3 c_VertexControls;
IN vec3 i_Pos;
IN vec3 i_Normal;
IN vec3 i_Specular;
IN vec2 i_BaseTexCoord;
OUT vec3 v_Color;

#ifdef DETAILTEXTURE
	OUT vec4 v_BaseDetailTexCoord;
	#if defined(SEPARATE_DETAIL_UVS)
		IN vec2 i_DetailTexCoord;
	#else
		uniform vec4 c_DetailTexCoordTransform[2];
	#endif
#else
	OUT vec2 v_BaseDetailTexCoord;
#endif

#ifdef CUBEMAP
	uniform vec3 c_EyePos;
	OUT vec3 v_WorldSpaceNormal, v_WorldVertToEyeVector;
#endif

#ifndef NOFOG
	OUT vec2 v_WorldProjPosZ;
#endif

void main()
{
	vec3 worldPos, worldNormal;
	SkinPositionAndNormal(c_VertexControls.x, i_Pos, i_Normal, worldPos, worldNormal);
	worldNormal = normalize(worldNormal);

	gl_Position = vec4(worldPos, 1.0) * s_ViewProj;
	#ifndef NOFOG
		v_WorldProjPosZ = vec2(worldPos.z, gl_Position.z);
	#endif

	#ifdef CUBEMAP
		v_WorldSpaceNormal = worldNormal;
		v_WorldVertToEyeVector = c_EyePos - worldPos;
	#endif

	v_Color = DoLighting(worldPos, worldNormal, mix(vec3(0.0), i_Specular.bgr, c_VertexControls.y), c_VertexControls.z);

	v_BaseDetailTexCoord.xy = Mul4x2(i_BaseTexCoord, c_BaseTexCoordTransform[0], c_BaseTexCoordTransform[1]);
	#if defined(SEPARATE_DETAIL_UVS)
		v_BaseDetailTexCoord.zw = i_DetailTexCoord;
	#elif defined(DETAILTEXTURE)
		v_BaseDetailTexCoord.zw = Mul4x2(i_BaseTexCoord, c_DetailTexCoordTransform[0], c_DetailTexCoordTransform[1]);
	#endif
}