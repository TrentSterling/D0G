uniform sampler2D t_Tex;
IN vec4 v_CoordTap01, v_CoordTap23;
void main()
{
	vec4 avgColor =
		texture2D(t_Tex, v_CoordTap01.xy) +
		texture2D(t_Tex, v_CoordTap01.zw) +
		texture2D(t_Tex, v_CoordTap23.xy) +
		texture2D(t_Tex, v_CoordTap23.zw);
	gl_FragColor = vec4(avgColor.rgb * 0.25, 1.0);
}