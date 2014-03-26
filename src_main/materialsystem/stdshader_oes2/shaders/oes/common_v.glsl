#if __VERSION__ >= 300
	#define IN in
	#define OUT out
#else
	#define IN attribute
	#define OUT varying
#endif
vec2 Mul4x2(vec2 v, vec4 r0, vec4 r1)
{
	vec4 v4 = vec4(v, 0.0, 1.0);
	return vec2(dot(v4, r0), dot(v4, r1));
}
vec3 Mul4x3(vec3 v, vec4 r0, vec4 r1, vec4 r2)
{
	vec4 v4 = vec4(v, 1.0);
	return vec3(dot(v4, r0), dot(v4, r1), dot(v4, r2));
}