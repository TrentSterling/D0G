IN vec3 i_Pos;
#ifdef USESCOLOR
	IN vec4 i_Color;
	OUT vec4 v_Color;
#endif
void main()
{
	gl_Position = vec4(i_Pos, 1.0);
	#ifdef USESCOLOR
		v_Color = i_Color.bgra;
	#endif
}