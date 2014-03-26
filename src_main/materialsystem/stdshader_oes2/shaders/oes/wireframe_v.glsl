uniform mat4 s_ViewProj;
uniform float c_Skinning;
IN vec3 i_Pos;
#ifdef VERTEXCOLOR
	IN vec4 i_Color;
	OUT vec3 v_Color;
#endif
void main()
{
	gl_Position = vec4(SkinPosition(c_Skinning, i_Pos), 1.0) * s_ViewProj;
	#ifdef VERTEXCOLOR
		v_Color = i_Color.rgb;
	#endif
}