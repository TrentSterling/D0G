uniform sampler2D t_Base, t_Alpha;
IN vec4 v_TexCoord;
#ifndef NOFOG
	uniform float c_EyePosZ;
	IN float v_WorldProjPosZ;
#endif
void main()
{
	vec4 textureColor = texture2D(t_Base, v_TexCoord.xy) * texture2D(t_Alpha, v_TexCoord.zw);
	#ifdef NOFOG
		gl_FragColor = textureColor;
	#else
		gl_FragColor = vec4(BlendPixelFog(textureColor.rgb, c_EyePosZ, v_WorldProjPosZ.x, v_WorldProjPosZ.y), textureColor.a);
	#endif
}