uniform vec4 c_PixelControls;
uniform vec4 c_DiffuseModulation;
uniform sampler2D t_Base;
IN vec3 v_Color;

#ifdef DETAILTEXTURE
	uniform vec3 c_DetailTint;
	uniform sampler2D t_Detail;
	IN vec4 v_BaseDetailTexCoord;
#else
	IN vec2 v_BaseDetailTexCoord;
#endif

#if defined(ENVMAPMASK) || defined(SELFILLUM_ENVMAPMASK_ALPHA)
	uniform sampler2D t_EnvmapMask;
#endif

#ifdef SELFILLUM
	uniform vec3 c_SelfIllumTint;
	#ifdef SELFILLUMMASK
		uniform sampler2D t_SelfIllumMask;
	#endif
#endif

#ifdef CUBEMAP
	uniform vec3 c_EnvmapTint, c_EnvmapSaturation;
	uniform samplerCube t_Envmap;
	IN vec3 v_WorldSpaceNormal, v_WorldVertToEyeVector;
#endif

#ifndef NOFOG
	IN vec2 v_WorldProjPosZ;
#endif

void main()
{
	vec4 baseColor = texture2D(t_Base, v_BaseDetailTexCoord.xy);

	#ifdef DETAILTEXTURE
		baseColor.rgb *= mix(vec3(1.0), 2.0 * texture2D(t_Detail, v_BaseDetailTexCoord.zw).rgb * c_DetailTint, c_PixelControls.x);
	#endif

	#if defined(BASEALPHAENVMAPMASK) || defined(SELFILLUM)
		vec4 result = vec4(baseColor.rgb, 1.0) * c_DiffuseModulation;
	#else
		vec4 result = baseColor * c_DiffuseModulation;
	#endif

	#ifdef ALPHATEST
		if (result.a < c_PixelControls.y)
			discard;
	#endif

	result.rgb *= v_Color;

	#if defined(ENVMAPMASK) || defined(SELFILLUM_ENVMAPMASK_ALPHA)
		vec4 envmapMaskTexel = texture2D(t_EnvmapMask, v_BaseTexCoord.xy);
	#endif

	#if defined(SELFILLUM_ENVMAPMASK_ALPHA)
		float adjAlpha = 8.0 * envmapMaskTexel.a;
		result.rgb = (max(0.0, 1.0 - adjAlpha) * result.rgb) + adjAlpha * c_SelfIllumTint * baseColor.rgb;
	#elif defined(SELFILLUMMASK)
		result.rgb = mix(result.rgb, c_SelfIllumTint * baseColor.rgb, texture2D(t_SelfIllumMask, v_BaseTexCoord.xy).rgb);
	#elif defined(SELFILLUM)
		result.rgb = mix(result.rgb, c_SelfIllumTint * baseColor.rgb, baseColor.a);
	#endif

	#ifdef CUBEMAP
		vec3 specularLighting = c_EnvmapTint * textureCube(t_Envmap,
			2.0 * dot(v_WorldSpaceNormal, v_WorldVertToEyeVector) * v_WorldSpaceNormal - dot(v_WorldSpaceNormal, v_WorldSpaceNormal) * v_WorldVertToEyeVector).rgb;
		#if defined(BASEALPHAENVMAPMASK)
			specularLighting *= 1.0 - baseColor.a;
		#elif defined(ENVMAPMASK)
			specularLighting *= envmapMaskTexel.xyz;
		#endif
		result.rgb += mix(vec3(dot(specularLighting, vec3(0.299, 0.587, 0.114))), specularLighting, c_EnvmapSaturation);
	#endif

	#ifdef NOFOG
		gl_FragColor = result;
	#else
		gl_FragColor = vec4(BlendPixelFog(result.rgb, c_PixelControls.z, v_WorldProjPosZ.x, v_WorldProjPosZ.y), result.a);
	#endif
}