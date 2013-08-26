IN vec3 vPos;
IN vec2 vTexCoord0;

OUT vec2 baseTexCoord;

void main()
{
	gl_Position = vec4(vPos, 1.0);
	baseTexCoord = vTexCoord0;
}