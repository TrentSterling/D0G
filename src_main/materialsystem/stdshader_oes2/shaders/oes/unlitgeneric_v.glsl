uniform mat4 s_ViewProj;
uniform float c_Skinning;
uniform vec4 c_BaseTexCoordTransform[2];
IN vec3 i_Pos;
IN vec2 i_BaseTexCoord;
#ifdef DETAILTEXTURE
	uniform vec4 c_DetailTexCoordTransform[2];
	#ifdef SEPARATE_DETAIL_UVS
		IN vec2 i_DetailTexCoord;
	#endif
	OUT vec2 v_DetailTexCoord;
#endif
#ifdef VERTEXCOLOR
	IN vec4 i_Color;
	OUT vec4 v_Color;
#endif
#ifdef NOFOG
	OUT vec2 v_BaseTexCoord_WorldProjPosZ;
#else
	OUT vec4 v_BaseTexCoord_WorldProjPosZ;
#endif
void main()
{
	vec3 worldPos = SkinPosition(c_Skinning, i_Pos);
	gl_Position = vec4(worldPos, 1.0) * s_ViewProj;
	#ifdef VERTEXCOLOR
		v_Color = i_Color.bgra;
	#endif
	v_BaseTexCoord_WorldProjPosZ.xy = Mul4x2(i_BaseTexCoord, c_BaseTexCoordTransform[0], c_BaseTexCoordTransform[1]);
	#ifndef NOFOG
		v_BaseTexCoord_WorldProjPosZ.zw = vec2(worldPos.z, gl_Position.z);
	#endif
	#if defined(SEPARATE_DETAIL_UVS)
		v_DetailTexCoord = i_DetailTexCoord;
	#elif defined(DETAILTEXTURE)
		v_DetailTexCoord = Mul4x2(i_BaseTexCoord, c_DetailTexCoordTransform[0], c_DetailTexCoordTransform[1]);
	#endif
}