DOG_C_DEFINES := \
  DEFINE_MATERIALSYSTEM_INTERFACE \
  MATERIALSYSTEM_EXPORTS
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  bitmap \
  mathlib \
  shaderlib \
  tier0 \
  tier1 \
  tier2 \
  vstdlib \
  vtf
DOG_SRC_FILES := \
  materialsystem/ccolorcorrection.cpp \
  materialsystem/cmaterial.cpp \
  materialsystem/cmaterial_queuefriendly.cpp \
  materialsystem/cmaterialdict.cpp \
  materialsystem/CMaterialSubRect.cpp \
  materialsystem/cmaterialsystem.cpp \
  materialsystem/cmaterialvar.cpp \
  materialsystem/cmatlightmaps.cpp \
  materialsystem/cmatnullrendercontext.cpp \
  materialsystem/cmatqueuedrendercontext.cpp \
  materialsystem/cmatrendercontext.cpp \
  materialsystem/colorspace.cpp \
  materialsystem/ctexture.cpp \
  public/filesystem_helpers.cpp \
  materialsystem/imagepacker.cpp \
  materialsystem/mat_stub.cpp \
  materialsystem/materialsystem_global.cpp \
  public/tier0/memoverride.cpp \
  materialsystem/morph.cpp \
  materialsystem/occlusionquerymgr.cpp \
  materialsystem/pch_materialsystem.cpp \
  materialsystem/shadersystem.cpp \
  materialsystem/texturemanager.cpp