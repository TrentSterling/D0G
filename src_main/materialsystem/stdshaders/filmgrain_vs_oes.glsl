IN vec3 vPos;
IN vec2 vTexCoord0;

OUT vec2 InputImageCoord;
OUT vec2 FilmGrainCoord;

uniform vec4 cFilmGrainOffset;

void main()
{
	gl_Position = vec4(vPos, 1.0);
	InputImageCoord = vTexCoord0;
	FilmGrainCoord = vTexCoord0 * cFilmGrainOffset.xy + cFilmGrainOffset.zw;
}