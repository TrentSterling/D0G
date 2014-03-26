#ifdef USESCOLOR
	IN vec4 v_Color;
#endif
void main()
{
	#ifdef USESCOLOR
		gl_FragColor = v_Color;
	#else
		gl_FragColor = vec4(0.0);
	#endif
}