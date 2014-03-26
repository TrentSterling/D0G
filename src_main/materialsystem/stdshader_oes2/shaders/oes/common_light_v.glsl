uniform vec4 s_AmbientCube[6];
uniform vec4 s_LightColor[4];
uniform vec4 s_LightDir[4];
uniform vec3 s_LightPos[4];
uniform vec4 s_LightSpot[4];
uniform vec3 s_LightAtten[4];
uniform int s_LightEnable;
vec3 AmbientLight(vec3 worldNormal)
{
	vec3 nSquared = worldNormal * worldNormal;
	bvec3 isNegative = lessThan(worldNormal, vec3(0.0));
	return nSquared.x * s_AmbientCube[int(isNegative.x)].rgb +
		nSquared.y * s_AmbientCube[2 + int(isNegative.y)].rgb +
		nSquared.z * s_AmbientCube[4 + int(isNegative.z)].rgb;
}
float VertexAttenInternal(vec3 worldPos, int lightNum)
{
	vec3 lightDir = s_LightPos[lightNum] - worldPos;
	float lightDistSquared = dot(lightDir, lightDir);
	float ooLightDist = inversesqrt(lightDistSquared);
	lightDir *= ooLightDist;
	float distanceAtten = 1.0 / dot(s_LightAtten[lightNum], vec3(1.0, lightDistSquared * ooLightDist, lightDistSquared));
	float spotAtten = clamp(pow(max(0.0001, (s_LightSpot[lightNum].z - dot(s_LightDir[lightNum].xyz, lightDir)) * s_LightSpot[lightNum].w),
		s_LightSpot[lightNum].x), 0.0, 1.0);
	return mix(mix(distanceAtten, distanceAtten * spotAtten, s_LightDir[lightNum].w), 1.0, s_LightColor[lightNum].a);
}
float CosineTermInternal(vec3 worldPos, vec3 worldNormal, int lightNum, float halfLambert)
{
	float nDotL = dot(worldNormal,
		mix(normalize(s_LightPos[lightNum] - worldPos), -(s_LightDir[lightNum].xyz), s_LightColor[lightNum].a));
	return mix(max(0.0, nDotL), (nDotL * 0.5 + 0.5) * (nDotL * 0.5 + 0.5), halfLambert);
}
vec3 DoLightInternal(vec3 worldPos, vec3 worldNormal, int lightNum, float halfLambert)
{
	return s_LightColor[lightNum].rgb *
		CosineTermInternal(worldPos, worldNormal, lightNum, halfLambert) *
		VertexAttenInternal(worldPos, lightNum);
}
vec3 DoLighting(vec3 worldPos, vec3 worldNormal, vec3 staticLight, float dynamicLight_halfLambertSign)
{
	vec3 col = staticLight * 2.0;
	if (dynamicLight_halfLambertSign != 0.0)
	{
		float halfLambert = step(dynamicLight_halfLambertSign, 0.0);
		if (s_LightEnable > 0)
		{
			col += DoLightInternal(worldPos, worldNormal, 0, halfLambert);
			if (s_LightEnable > 1)
				col += DoLightInternal(worldPos, worldNormal, 1, halfLambert);
		}
		col += AmbientLight(worldNormal);
	}
	return col;
}