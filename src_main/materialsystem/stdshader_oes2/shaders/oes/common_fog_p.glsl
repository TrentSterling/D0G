uniform vec4 c_FogParams[2];
vec3 BlendPixelFog(vec3 shaderColor, float eyePosZ, float worldPosZ, float projPosZ)
{
	float linearFogFactor = clamp(c_FogParams[0].x + projPosZ * c_FogParams[0].w, 0.0, c_FogParams[0].z);
	float waterFogFactor = clamp(((c_FogParams[0].y - worldPosZ) / (eyePosZ - worldPosZ)) * projPosZ * c_FogParams[0].w, 0.0, 1.0);
	return mix(shaderColor, c_FogParams[1].rgb, mix(waterFogFactor, linearFogFactor, step(0.0, c_FogParams[0].z)));
}