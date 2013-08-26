#define saturate(x) clamp(x, 0.0, 1.0)

vec3 WorldToTangent(vec3 vector, vec3 normal, vec3 tangent, vec3 binormal)
{
	return vec3(dot(vector, tangent), dot(vector, binormal), dot(vector, normal));
}

vec3 TangentToWorld(vec3 vector, vec3 normal, vec3 tangent, vec3 binormal)
{
	return vector.x * tangent + vector.y * binormal + vector.z * normal;
}