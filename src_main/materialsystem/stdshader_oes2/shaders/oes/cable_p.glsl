uniform sampler2D t_Normal, t_Base;
IN vec4 v_TexCoord;
IN vec4 v_DirectionalLightColor;
#ifndef NOFOG
	uniform float c_EyePosZ;
	IN vec2 v_WorldProjPosZ;
#endif
void main()
{
	vec4 textureColor = texture2D(t_Base, v_TexCoord.zw) * v_DirectionalLightColor;
	#ifdef ALPHATEST
		if (textureColor.a < 0.7)
			discard;
	#endif
	float lightDirDotNormalMap = texture2D(t_Normal, v_TexCoord.xy).z;
	#ifdef NOFOG
		gl_FragColor = vec4(lightDirDotNormalMap * lightDirDotNormalMap * textureColor.rgb, textureColor.a);
	#else
		gl_FragColor = vec4(
			BlendPixelFog(lightDirDotNormalMap * lightDirDotNormalMap * textureColor.rgb, c_EyePosZ, v_WorldProjPosZ.x, v_WorldProjPosZ.y),
			textureColor.a);
	#endif
}