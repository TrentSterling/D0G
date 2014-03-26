uniform mat4 s_ViewProj;
uniform vec4 s_Model[3];
IN vec3 i_Pos;
IN vec2 i_TexCoord0, i_TexCoord1;
IN vec4 i_DirectionalLightColor;
OUT vec4 v_TexCoord;
OUT vec4 v_DirectionalLightColor;
#ifndef NOFOG
	OUT vec2 v_WorldProjPosZ;
#endif
void main()
{
	vec3 worldPos = Mul4x3(i_Pos, s_Model[0], s_Model[1], s_Model[2]);
	gl_Position = vec4(worldPos, 1.0) * s_ViewProj;
	#ifndef NOFOG
		v_WorldProjPosZ = vec2(worldPos.z, gl_Position.z);
	#endif
	v_TexCoord = vec4(i_TexCoord0, i_TexCoord1);
	v_DirectionalLightColor = i_DirectionalLightColor.bgra;
}