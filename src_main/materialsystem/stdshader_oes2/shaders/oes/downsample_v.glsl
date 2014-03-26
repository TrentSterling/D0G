uniform vec4 c_TapOffs[2];
IN vec3 i_Pos;
IN vec2 i_BaseTexCoord;
OUT vec4 v_CoordTap01, v_CoordTap23;
void main()
{
	gl_Position = vec4(i_Pos, 1.0);
	vec4 baseTexCoord = vec4(i_BaseTexCoord, i_BaseTexCoord);
	v_CoordTap01 = baseTexCoord + c_TapOffs[0];
	v_CoordTap23 = baseTexCoord + c_TapOffs[1];
}