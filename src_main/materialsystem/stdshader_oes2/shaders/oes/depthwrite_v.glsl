uniform mat4 s_ViewProj;
uniform float c_Skinning;
IN vec3 i_Pos;
#ifdef ALPHATEST
	IN vec2 i_BaseTexCoord;
	OUT vec2 v_BaseTexCoord;
#endif
void main()
{
	gl_Position = vec4(SkinPosition(c_Skinning, i_Pos), 1.0) * s_ViewProj;
	#ifdef ALPHATEST
		v_BaseTexCoord = i_BaseTexCoord;
	#endif
}