DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1" \
  "$(DOG_SRC_MAIN)/materialsystem"
DOG_SRC_FILES := \
  materialsystem/shaderlib/BaseShader.cpp \
  materialsystem/shaderlib/ShaderDLL.cpp \
  materialsystem/shaderlib/shaderlib_cvar.cpp
DOG_STATIC_LIBRARY := true