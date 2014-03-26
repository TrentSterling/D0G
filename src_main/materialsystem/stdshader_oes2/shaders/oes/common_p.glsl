precision mediump float;
#if __VERSION__ >= 300
	#define IN in
#else
	#define IN varying
	#ifndef GL_FRAGMENT_PRECISION_HIGH
		#define highp
	#endif
#endif