uniform vec4 s_Model[48];
IN vec2 i_BoneWeights;
IN vec3 i_BoneIndices;
void SkinBlendMatrix(float skinning, out vec4 r0, out vec4 r1, out vec4 r2)
{
	if (skinning == 0.0)
	{
		r0 = s_Model[0];
		r1 = s_Model[1];
		r2 = s_Model[2];
		return;
	}
	ivec3 boneIndices = ivec3(i_BoneIndices) * 3;
	vec3 weights = vec3(i_BoneWeights, 1.0 - i_BoneWeights.x - i_BoneWeights.y);
	r0 = s_Model[boneIndices.x] * weights.x + s_Model[boneIndices.y] * weights.y + s_Model[boneIndices.z] * weights.z;
	r1 = s_Model[boneIndices.x + 1] * weights.x + s_Model[boneIndices.y + 1] * weights.y + s_Model[boneIndices.z + 1] * weights.z;
	r2 = s_Model[boneIndices.x + 2] * weights.x + s_Model[boneIndices.y + 2] * weights.y + s_Model[boneIndices.z + 2] * weights.z;
}
vec3 SkinPosition(float skinning, vec3 modelPos)
{
	vec4 m0, m1, m2;
	SkinBlendMatrix(skinning, m0, m1, m2);
	return Mul4x3(modelPos, m0, m1, m2);
}
void SkinPositionAndNormal(float skinning, vec3 modelPos, vec3 modelNormal, out vec3 worldPos, out vec3 worldNormal)
{
	vec4 m0, m1, m2;
	SkinBlendMatrix(skinning, m0, m1, m2);
	worldPos = Mul4x3(modelPos, m0, m1, m2);
	worldNormal = vec3(dot(modelNormal, m0.xyz), dot(modelNormal, m1.xyz), dot(modelNormal, m2.xyz));
}
void SkinPositionNormalAndTangentSpace(float skinning,
	vec3 modelPos, vec3 modelNormal, vec4 modelTangentS, vec3 modelTangentT,
	out vec3 worldPos, out vec3 worldNormal, out vec3 worldTangentS, out vec3 worldTangentT)
{
	vec4 m0, m1, m2;
	SkinBlendMatrix(skinning, m0, m1, m2);
	worldPos = Mul4x3(modelPos, m0, m1, m2);
	worldNormal = vec3(dot(modelNormal, m0.xyz), dot(modelNormal, m1.xyz), dot(modelNormal, m2.xyz));
	worldTangentS = vec3(dot(modelTangentS.xyz, m0.xyz), dot(modelTangentS.xyz, m1.xyz), dot(modelTangentS.xyz, m2.xyz));
	worldTangentT = cross(worldNormal, worldTangentS) * modelTangentS.w;
}