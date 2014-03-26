#ifdef ALPHATEST
	uniform float c_AlphaThreshold;
	uniform sampler2D t_Base;
	IN vec2 v_BaseTexCoord;
#endif
void main()
{
	#ifdef ALPHATEST
		if (texture2D(t_Base, v_BaseTexCoord).a < c_AlphaThreshold)
			discard;
	#endif
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}