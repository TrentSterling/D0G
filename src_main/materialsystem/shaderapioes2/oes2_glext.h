//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Used OpenGL ES 2.0 extensions.
//
//===========================================================================//
#ifndef OES2_GLEXT_H
#define OES2_GLEXT_H

#include <KHR/khrplatform.h>

//-----------------------------------------------------------------------------
// Functions must be renamed from "glX" to "glextX" to avoid collisions.
// Things that are commented out are not used in Source.
// "OES3 standard" means that the extension is in the core OpenGL ES 3.0.
//-----------------------------------------------------------------------------

// EXT_occlusion_query_boolean (OES3 standard)
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT	0x8D6A
#define GL_QUERY_RESULT_EXT						0x8866
#define GL_QUERY_RESULT_AVAILABLE_EXT			0x8867
#ifndef SHADERAPIOES3
typedef void (KHRONOS_APIENTRY *PFNGLGENQUERIESEXTPROC)(int n, unsigned int *ids);
extern PFNGLGENQUERIESEXTPROC glextGenQueriesEXT;
#define glGenQueries (*glextGenQueriesEXT)
typedef void (KHRONOS_APIENTRY *PFNGLDELETEQUERIESEXTPROC)(int n, const unsigned int *ids);
extern PFNGLDELETEQUERIESEXTPROC glextDeleteQueriesEXT;
#define glDeleteQueries (*glextDeleteQueriesEXT)
typedef void (KHRONOS_APIENTRY *PFNGLBEGINQUERYEXTPROC)(unsigned int target, unsigned int id);
extern PFNGLBEGINQUERYEXTPROC glextBeginQueryEXT;
#define glBeginQuery (*glextBeginQueryEXT)
typedef void (KHRONOS_APIENTRY *PFNGLENDQUERYEXTPROC)(unsigned int target);
extern PFNGLENDQUERYEXTPROC glextEndQueryEXT;
#define glEndQuery (*glextEndQueryEXT)
typedef void (KHRONOS_APIENTRY *PFNGLGETQUERYOBJECTUIVEXTPROC)(unsigned int id, unsigned int pname, unsigned int *params);
extern PFNGLGETQUERYOBJECTUIVEXTPROC glextGetQueryObjectuivEXT;
#define glGetQueryObjectuiv (*glextGetQueryObjectuivEXT)
#endif

// EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT			0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT		0x84FF

// NV_occlusion_query_samples
#define GL_SAMPLES_PASSED_NV					0x88BF

// NV_texture_border_clamp
#define GL_CLAMP_TO_BORDER_NV					0x812D

// NV_texture_compression_s3tc
#define GL_COMPRESSED_RGB_S3TC_DXT1_NV			0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_NV			0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_NV			0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_NV			0x83F3

// OES_depth24 (OES3 standard)
#define GL_DEPTH_COMPONENT24_OES				0x81A6

// OES_packed_depth_stencil (OES3 standard)
#define GL_DEPTH_STENCIL_OES					0x84F9
#define GL_UNSIGNED_INT_24_8_OES				0x84FA
#define GL_DEPTH24_STENCIL8_OES					0x88F0

// OES_texture_3D (OES3 standard)
#define GL_TEXTURE_3D_OES						0x806F
#define GL_TEXTURE_WRAP_R_OES					0x8072
#define GL_MAX_3D_TEXTURE_SIZE_OES				0x8073
#ifndef SHADERAPIOES3
typedef void (KHRONOS_APIENTRY *PFNGLTEXIMAGE3DOESPROC)(unsigned int target, int level, unsigned int internalFormat,
	int width, int height, int depth, int border, unsigned int format, unsigned int type, const void *pixels);
extern PFNGLTEXIMAGE3DOESPROC glextTexImage3DOES;
#define glTexImage3D (*glextTexImage3DOES)
typedef void (KHRONOS_APIENTRY *PFNGLTEXSUBIMAGE3DOESPROC)(unsigned int target, int level,
	int xoffset, int yoffset, int zoffset, int width, int height, int depth,
	unsigned int format, unsigned int type, const void *pixels);
extern PFNGLTEXSUBIMAGE3DOESPROC glextTexSubImage3DOES;
#define glTexSubImage3D (*glextTexSubImage3DOES)
#endif

#endif // !OES2_GLEXT_H