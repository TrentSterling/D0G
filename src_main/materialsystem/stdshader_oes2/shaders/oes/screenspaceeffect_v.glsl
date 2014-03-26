IN vec3 i_Pos;
IN vec2 i_BaseTexCoord;
OUT vec2 v_BaseTexCoord;
void main()
{
	gl_Position = vec4(iPos, 1.0);
	v_BaseTexCoord = i_BaseTexCoord;
}