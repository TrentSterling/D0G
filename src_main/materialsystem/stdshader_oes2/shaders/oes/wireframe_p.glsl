#ifdef VERTEXCOLOR
	IN vec3 v_Color;
#endif
void main()
{
	#ifdef VERTEXCOLOR
		gl_FragColor = vec4(v_Color, 1.0);
	#else
		gl_FragColor = vec4(1.0);
	#endif
}