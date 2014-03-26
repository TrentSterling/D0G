uniform mat4 s_ModelViewProj;
uniform vec4 c_BaseTextureTransform[2], c_MaskTransform[2];
IN vec3 i_Pos;
IN vec2 i_TexCoord0, i_TexCoord1;
OUT vec4 v_TexCoord;
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
	v_TexCoord.xy = Mul4x2(i_TexCoord0, c_BaseTextureTransform[0], c_BaseTextureTransform[1]);
	v_TexCoord.zw = Mul4x2(i_TexCoord1, c_MaskTransform[0], c_MaskTransform[1]);
}