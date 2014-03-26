//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 shader management.
//
//===========================================================================//
#include <cstd/string.h>
#include "filesystem.h"
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Public interface.
//-----------------------------------------------------------------------------

void CShaderAPIOES2::BindShaderProgram(ShaderProgramHandle_t program)
{
	if ((m_BoundProgram == program) || !ProgramIsAllocated(program))
		return;
	m_BoundProgram = program;
	ShaderProgram_t &p = m_Programs[program];
	m_BoundProgramConstants = p.m_Constants;
	if (!IsDeactivated())
		glUseProgram(p.m_Program);
}

void CShaderAPIOES2::ClearVertexAndPixelShaderRefCounts(void)
{
	ShaderProgramHandle_t hProgram;
	for (hProgram = m_Programs.Head(); hProgram != m_Programs.InvalidIndex(); hProgram = m_Programs.Next(hProgram))
		m_Programs[hProgram].m_RefCount = 0;
}

ShaderProgramHandle_t CShaderAPIOES2::GetShaderProgram(const char *pName, int combo)
{
	CRC32_t name = CRC32_ProcessSingleBuffer(pName, strlen(pName));
	ShaderProgramHandle_t hProgram;
	ShaderProgram_t *p;
	for (hProgram = m_Programs.Head(); hProgram != m_Programs.InvalidIndex(); hProgram = m_Programs.Next(hProgram))
	{
		p = &(m_Programs[hProgram]);
		if (p->m_Program && (p->m_Name == name) && (p->m_Combo == combo))
		{
			++(p->m_RefCount);
			return hProgram;
		}
	}
	return SHADER_PROGRAM_HANDLE_INVALID;
}

void CShaderAPIOES2::PurgeUnusedVertexAndPixelShaders(void)
{
	bool active = !(IsDeactivated());
	ShaderProgramHandle_t hProgram;
	ShaderProgram_t *p;
	for (hProgram = m_Programs.Head(); hProgram != m_Programs.InvalidIndex(); hProgram = m_Programs.Next(hProgram))
	{
		p = &(m_Programs[hProgram]);
		if ((p->m_RefCount > 0) || !(p->m_Program))
			continue;
		if (active)
		{
			glDeleteProgram(p->m_Program);
			glDeleteShader(p->m_VertexShader);
			glDeleteShader(p->m_PixelShader);
		}
		p->m_Program = 0;
	}
}

void CShaderAPIOES2::Uniform1f(int location, float v0)
	{ glUniform1f(m_BoundProgramConstants[location], v0); }
void CShaderAPIOES2::Uniform2f(int location, float v0, float v1)
	{ glUniform2f(m_BoundProgramConstants[location], v0, v1); }
void CShaderAPIOES2::Uniform3f(int location, float v0, float v1, float v2)
	{ glUniform3f(m_BoundProgramConstants[location], v0, v1, v2); }
void CShaderAPIOES2::Uniform4f(int location, float v0, float v1, float v2, float v3)
	{ glUniform4f(m_BoundProgramConstants[location], v0, v1, v2, v3); }

void CShaderAPIOES2::Uniform1i(int location, int v0)
	{ glUniform1i(m_BoundProgramConstants[location], v0); }
void CShaderAPIOES2::Uniform2i(int location, int v0, int v1)
	{ glUniform2i(m_BoundProgramConstants[location], v0, v1); }
void CShaderAPIOES2::Uniform3i(int location, int v0, int v1, int v2)
	{ glUniform3i(m_BoundProgramConstants[location], v0, v1, v2); }
void CShaderAPIOES2::Uniform4i(int location, int v0, int v1, int v2, int v3)
	{ glUniform4i(m_BoundProgramConstants[location], v0, v1, v2, v3); }

void CShaderAPIOES2::Uniform1fv(int location, unsigned int count, const float *value)
	{ glUniform1fv(m_BoundProgramConstants[location], count, value); }
void CShaderAPIOES2::Uniform2fv(int location, unsigned int count, const float *value)
	{ glUniform2fv(m_BoundProgramConstants[location], count, value); }
void CShaderAPIOES2::Uniform3fv(int location, unsigned int count, const float *value)
	{ glUniform3fv(m_BoundProgramConstants[location], count, value); }
void CShaderAPIOES2::Uniform4fv(int location, unsigned int count, const float *value)
	{ glUniform4fv(m_BoundProgramConstants[location], count, value); }

void CShaderAPIOES2::UniformMatrix2fv(int location, unsigned int count, const float *value)
	{ glUniformMatrix2fv(m_BoundProgramConstants[location], count, GL_FALSE, value); }
void CShaderAPIOES2::UniformMatrix3fv(int location, unsigned int count, const float *value)
	{ glUniformMatrix3fv(m_BoundProgramConstants[location], count, GL_FALSE, value); }
void CShaderAPIOES2::UniformMatrix4fv(int location, unsigned int count, const float *value)
	{ glUniformMatrix4fv(m_BoundProgramConstants[location], count, GL_FALSE, value); }

//-----------------------------------------------------------------------------
// Private API functions.
//-----------------------------------------------------------------------------

bool CShaderAPIOES2::CommitStandardConstant(int index)
{
	if (!IsProgramBound())
		return false;
	ShaderProgram_t::StandardConstant_t &constant = GetBoundProgram().m_StandardConstants[index];
	if (constant.m_Location < 0)
		return false;
	if ((constant.m_UpdateFrame == m_StandardConstUpdateFrame[index]) &&
		(constant.m_Update == m_StandardConstUpdate[index]))
		return false;
	constant.m_UpdateFrame = m_StandardConstUpdateFrame[index];
	constant.m_Update = m_StandardConstUpdate[index];
	return true;
}

//-----------------------------------------------------------------------------
// Program loader.
//-----------------------------------------------------------------------------

#define SHADER_SOURCE_PATH "shaders/oes/"

unsigned int CShaderAPIOES2::LoadOESShader(unsigned int type, const char * const *files, int nFiles, int version)
{
	Assert(nFiles <= OES2_SHADER_MAX_FILES);

	char path[MAX_PATH];
	strcpy(path, SHADER_SOURCE_PATH);
	char *pPath = path + sizeof(SHADER_SOURCE_PATH) - 1;

	const char *sourcesBuffer[OES2_SHADER_MAX_FILES + 1];

	int headerCount = 0;
#ifdef SHADERAPIOES3
	if (version == OES2_SHADER_LANGUAGE_ESSL3)
		sourcesBuffer[headerCount++] = "#version 300\n";
	else
#endif
		Assert(version == OES2_SHADER_LANGUAGE_ESSL2);

	unsigned int shader = 0;

	const char * const *pFile = files;
	int nFile; // Must be preserved for release.
	const char **sources = sourcesBuffer + headerCount;
	for (nFile = 0; nFile < nFiles; ++nFile, ++pFile)
	{
		if (**pFile == '.') // Local file.
		{
			strcpy(pPath, *pFile + 1);
			strcat(pPath, ".glsl");
			FileHandle_t fileHandle = g_pFullFileSystem->Open(path, "rb");
			if (fileHandle == FILESYSTEM_INVALID_HANDLE)
			{
				Warning("CShaderAPIOES2::LoadProgramSource: Can't open shader source file \"%s\".\n", *pFile + 1);
				goto release;
			}
			int size = g_pFullFileSystem->Size(fileHandle);
			char *file = new char[size + 2];
			g_pFullFileSystem->Read(file, size, fileHandle);
			g_pFullFileSystem->Close(fileHandle);
			file[size] = '\n';
			file[size + 1] = '\0';
			sources[nFile] = file;
		}
		else if (**pFile == '/') // Global include.
		{
			const char *name = *pFile + 1;
			int index = m_GlobalProgramSources.Find(name);
			if (index != m_GlobalProgramSources.InvalidIndex())
			{
				sources[nFile] = m_GlobalProgramSources[index];
			}
			else
			{
				strcpy(pPath, name);
				strcat(pPath, ".glsl");
				FileHandle_t fileHandle = g_pFullFileSystem->Open(path, "rb");
				if (fileHandle == FILESYSTEM_INVALID_HANDLE)
				{
					Warning("CShaderAPIOES2::LoadProgramSource: Can't open shader include file \"%s\".\n", name);
					goto release;
				}
				int size = g_pFullFileSystem->Size(fileHandle);
				char *file = new char[size + 2]; // Must not goto release after `new` this iteration!
				g_pFullFileSystem->Read(file, size, fileHandle);
				g_pFullFileSystem->Close(fileHandle);
				file[size] = '\n';
				file[size + 1] = '\0';
				m_GlobalProgramSources.Insert(name, file);
				sources[nFile] = file;
			}
		}
		else // Direct inclusion from the code. Must end with \n.
		{
			sources[nFile] = *pFile;
		}
	}

	shader = glCreateShader(type);
	if (!shader)
	{
		Warning("CShaderAPIOES2::LoadProgramSource: glCreateShader failed: %s.\n", OESErrorString());
		goto release;
	}
	glShaderSource(shader, headerCount + nFiles, sourcesBuffer, NULL);
	glCompileShader(shader);
	{
		int compileStatus;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
		if (!compileStatus)
		{
			int length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			char *log = (char *)(stackalloc(length + 1));
			glGetShaderInfoLog(shader, length, NULL, log);
			log[length] = '\0';
			Warning("CShaderAPIOES2::LoadProgramSource: glCompileShader failed: %s.\n", log);
			goto fail;
		}
	}

	goto release;

fail:
	glDeleteShader(shader);
	shader = 0;
release:
	for (; nFile-- > 0; )
	{
		if (files[nFile][0] == '.')
			delete[] sources[nFile];
	}
	return shader;
}

static void CreateShaderProgram_StandardConstants(unsigned int program, OES2ShaderConstantUsage_t usage,
	ShaderProgram_t::StandardConstant_t *constants)
{
	// Mapped to OES2_SHADER_CONST_ enum!!!
	const char *names[] =
	{
		"s_ModelViewProj",
		"s_ViewProj",
		"s_ViewModel",
		"s_AmbientCube",
		"s_LightColor",
		"s_LightDir",
		"s_LightPos",
		"s_LightSpot",
		"s_LightAtten",
		"s_LightEnable",
		"s_Model"
	};
	int i;
	for (i = OES2_SHADER_CONST_COUNT; i--; )
	{
		if (usage & (1 << i))
			constants[i].m_Location = glGetUniformLocation(program, names[i]);
	}
}

static void CreateShaderProgram_VertexUsage(OES2ShaderCreationData_t &data, ShaderProgram_t &p)
{
	int i;
	for (i = OES2_SHADER_INPUT_COUNT; i--; )
	{
		if (data.m_Attributes[i])
			p.m_VertexInputs[i] = glGetAttribLocation(p.m_Program, data.m_Attributes[i]);
	}

	p.m_VertexUsage = 0;

	if (p.m_VertexInputs[OES2_SHADER_INPUT_POSITION] >= 0)
		p.m_VertexUsage |= VERTEX_POSITION;
	if (p.m_VertexInputs[OES2_SHADER_INPUT_BONE_WEIGHT] >= 0)
		p.m_VertexUsage |= VERTEX_BONEWEIGHT(2);
	if (p.m_VertexInputs[OES2_SHADER_INPUT_BONE_INDEX] >= 0)
		p.m_VertexUsage |= VERTEX_BONE_INDEX;
	if (p.m_VertexInputs[OES2_SHADER_INPUT_NORMAL] >= 0)
		p.m_VertexUsage |= VERTEX_NORMAL;
	if (p.m_VertexInputs[OES2_SHADER_INPUT_COLOR] >= 0)
		p.m_VertexUsage |= VERTEX_COLOR;
	if (p.m_VertexInputs[OES2_SHADER_INPUT_SPECULAR] >= 0)
		p.m_VertexUsage |= VERTEX_SPECULAR;
	if (p.m_VertexInputs[OES2_SHADER_INPUT_TANGENT_S] >= 0)
		p.m_VertexUsage |= VERTEX_TANGENT_S;
	if (p.m_VertexInputs[OES2_SHADER_INPUT_TANGENT_T] >= 0)
		p.m_VertexUsage |= VERTEX_TANGENT_T;
	if (p.m_VertexInputs[OES2_SHADER_INPUT_USERDATA] >= 0)
	{
		Assert(data.m_UserDataSize <= 4);
		p.m_VertexUsage |= VERTEX_USERDATA_SIZE(data.m_UserDataSize);
	}

	COMPILE_TIME_ASSERT(VERTEX_MAX_TEXTURE_COORDINATES == 8);
	for (i = VERTEX_MAX_TEXTURE_COORDINATES; i--; )
	{
		if (p.m_VertexInputs[OES2_SHADER_INPUT_TEXCOORD0 + i] >= 0)
		{
			Assert(data.m_TexCoordSizes[i] <= 4);
			p.m_VertexUsage |= VERTEX_TEXCOORD_SIZE(i, data.m_TexCoordSizes[i]);
		}
	}

	p.SetStreamOps();
}

ShaderProgramHandle_t CShaderAPIOES2::CreateShaderProgram(const char *pName, int combo, int format, void *pData)
{
	unsigned int vertexShader, pixelShader, program, oldProgram;
	OES2ShaderCreationData_t *data;
	ShaderProgram_t *p = NULL;
	ShaderProgramHandle_t hProgram;
	int i;

	if (format != OES2_SHADER_FORMAT)
	{
		Warning("CShaderAPIOES2::CreateShaderProgram: Invalid program data format specified for %s.\n", pName);
		goto fail_init;
	}
	if (IsDeactivated())
		goto fail_init;

	data = (OES2ShaderCreationData_t *)pData;
	Assert((data->m_VertexCount <= OES2_SHADER_MAX_FILES) && (data->m_PixelCount <= OES2_SHADER_MAX_FILES));

	vertexShader = LoadOESShader(GL_VERTEX_SHADER, data->m_VertexFiles, data->m_VertexCount, data->m_Language);
	if (!vertexShader)
		goto fail_init;
	pixelShader = LoadOESShader(GL_FRAGMENT_SHADER, data->m_PixelFiles, data->m_PixelCount, data->m_Language);
	if (!pixelShader)
		goto fail_ps;

	program = glCreateProgram();
	if (!program)
	{
		Warning("CShaderAPIOES2::CreateShaderProgram: glCreateProgram failed for %s: %s.\n", pName, OESErrorString());
		goto fail_pcreate;
	}
	glAttachShader(program, vertexShader);
	glAttachShader(program, pixelShader);
	glLinkProgram(program);
	// Must not detach/delete shaders here. Maybe it's ok to do so on desktop, but on mobile, we can't rely on
	// standards. Also, PerfHUD ES frame debugger crashes when attached shaders are deleted.
	{
		int linkStatus;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (!linkStatus)
		{
			int length;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			char *log = (char *)(stackalloc(length + 1));
			glGetProgramInfoLog(program, length, NULL, log);
			log[length] = '\0';
			Warning("CShaderAPIOES2::CreateShaderProgram: glLinkProgram failed for %s: %s.\n", pName, log);
			goto fail_plink;
		}
	}

	oldProgram = ProgramIsAllocated(m_BoundProgram) ? GetBoundProgram().m_Program : 0;
	glUseProgram(program);

	for (hProgram = m_Programs.Head(); hProgram != m_Programs.InvalidIndex(); hProgram = m_Programs.Next(hProgram))
	{
		p = &(m_Programs[hProgram]);
		if (!(p->m_Program)) // Find first free.
			break;
		p = NULL;
	}
	if (!p)
	{
		hProgram = m_Programs.AddToTail();
		p = &(m_Programs[hProgram]);
	}

	p->Defaults();
	p->m_Name = CRC32_ProcessSingleBuffer(pName, strlen(pName));
	p->m_Combo = combo;
	p->m_Program = program;
	p->m_VertexShader = vertexShader;
	p->m_PixelShader = pixelShader;
	CreateShaderProgram_StandardConstants(program, data->m_StandardConstants, p->m_StandardConstants);
	for (i = OES2_SHADER_MAX_CONSTANTS; i--; )
	{
		if (data->m_Constants[i])
			p->m_Constants[i] = glGetUniformLocation(program, data->m_Constants[i]);
	}
	// Ignore samplers 8-15 when unsupported - let shaders use them conditionally using caps only if they're supported.
	for (i = HardwareConfig()->Caps().m_NumCombinedSamplers; i-- > 0; )
	{
		if (data->m_Samplers[i])
			glUniform1i(glGetUniformLocation(program, data->m_Samplers[i]), i);
	}
	CreateShaderProgram_VertexUsage(*data, *p);
	p->m_RefCount = 1;

	if (oldProgram)
		glUseProgram(oldProgram);
	return hProgram;

fail_plink:
	glDeleteProgram(program);
fail_pcreate:
	glDeleteShader(pixelShader);
fail_ps:
	glDeleteShader(vertexShader);
fail_init:
	return SHADER_PROGRAM_HANDLE_INVALID;
}