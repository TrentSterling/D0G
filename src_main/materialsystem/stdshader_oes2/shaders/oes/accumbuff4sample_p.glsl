uniform vec4 c_Weights;
uniform sampler2D t_Tex0, t_Tex1, t_Tex2, t_Tex3;
IN vec2 v_BaseTexCoord;
void main()
{
	gl_FragColor =
		c_Weights.x * texture2D(t_Tex0, v_BaseTexCoord) +
		c_Weights.y * texture2D(t_Tex1, v_BaseTexCoord) +
		c_Weights.z * texture2D(t_Tex2, v_BaseTexCoord) +
		c_Weights.w * texture2D(t_Tex3, v_BaseTexCoord);
}