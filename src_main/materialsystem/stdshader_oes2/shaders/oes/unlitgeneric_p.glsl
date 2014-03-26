uniform vec4 c_PixelControls;
uniform sampler2D t_Base;
#ifdef DETAILTEXTURE
	uniform sampler2D t_Detail;
	IN vec2 v_DetailTexCoord;
#endif
#ifdef VERTEXCOLOR
	IN vec4 v_Color;
#endif
#ifdef NOFOG
	IN vec2 v_BaseTexCoord_WorldProjPosZ;
#else
	IN vec4 v_BaseTexCoord_WorldProjPosZ;
#endif
void main()
{
	vec4 baseColor = texture2D(t_Base, v_BaseTexCoord_WorldProjPosZ.xy);
	#ifdef DETAILTEXTURE
		baseColor.rgb *= mix(vec3(1.0), 2.0 * texture2D(t_Detail, v_DetailTexCoord).rgb, c_PixelControls.x);
	#endif
	#ifdef VERTEXCOLOR
		baseColor = vec4(baseColor.rgb * v_Color.rgb, mix(baseColor.a, baseColor.a * v_Color.a, c_PixelControls.y));
	#endif
	#ifdef ALPHATEST
		if (baseColor.a < c_PixelControls.z)
			discard;
	#endif
	#ifdef NOFOG
		gl_FragColor = baseColor;
	#else
		gl_FragColor = vec4(BlendPixelFog(baseColor.rgb, c_PixelControls.w, v_BaseTexCoord_WorldProjPosZ.z, v_BaseTexCoord_WorldProjPosZ.w), baseColor.a);
	#endif
}