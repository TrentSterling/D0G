DOG_C_DEFINES := FAST_MATERIALVAR_ACCESS
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  mathlib \
  shaderlib \
  tier0 \
  tier1 \
  vstdlib
DOG_SRC_FILES := \
  public/tier0/memoverride.cpp \
  materialsystem/stdshader_oes2/accumbuff4sample.cpp \
  materialsystem/stdshader_oes2/accumbuff5sample.cpp \
  materialsystem/stdshader_oes2/BaseOES2Shader.cpp \
  materialsystem/stdshader_oes2/bufferclearobeystencil.cpp \
  materialsystem/stdshader_oes2/cable.cpp \
  materialsystem/stdshader_oes2/cloud.cpp \
  materialsystem/stdshader_oes2/downsample.cpp \
  materialsystem/stdshader_oes2/unlitgeneric.cpp \
  materialsystem/stdshader_oes2/vertexlitgeneric.cpp