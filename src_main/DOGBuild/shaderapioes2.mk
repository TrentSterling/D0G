DOG_C_DEFINES := SHADER_DLL_EXPORT
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  bitmap \
  mathlib \
  tier0 \
  tier1 \
  tier2 \
  vstdlib
DOG_SRC_FILES := \
  public/filesystem_helpers.cpp \
  public/tier0/memoverride.cpp \
  materialsystem/shaderapioes2/oes2_api.cpp \
  materialsystem/shaderapioes2/oes2_colorformat.cpp \
  materialsystem/shaderapioes2/oes2_context.cpp \
  materialsystem/shaderapioes2/oes2_debugtextureinfo.cpp \
  materialsystem/shaderapioes2/oes2_devmgr.cpp \
  materialsystem/shaderapioes2/oes2_fog.cpp \
  materialsystem/shaderapioes2/oes2_framebuffer.cpp \
  materialsystem/shaderapioes2/oes2_glext.cpp \
  materialsystem/shaderapioes2/oes2_hardwareconfig.cpp \
  materialsystem/shaderapioes2/oes2_light.cpp \
  materialsystem/shaderapioes2/oes2_matrix.cpp \
  materialsystem/shaderapioes2/oes2_mesh_api.cpp \
  materialsystem/shaderapioes2/oes2_mesh_base.cpp \
  materialsystem/shaderapioes2/oes2_mesh_buffered.cpp \
  materialsystem/shaderapioes2/oes2_mesh_dynamic.cpp \
  materialsystem/shaderapioes2/oes2_mesh_ib.cpp \
  materialsystem/shaderapioes2/oes2_mesh_mgr.cpp \
  materialsystem/shaderapioes2/oes2_mesh_static.cpp \
  materialsystem/shaderapioes2/oes2_mesh_temp.cpp \
  materialsystem/shaderapioes2/oes2_mesh_vb.cpp \
  materialsystem/shaderapioes2/oes2_occlusionquery.cpp \
  materialsystem/shaderapioes2/oes2_selection.cpp \
  materialsystem/shaderapioes2/oes2_shader.cpp \
  materialsystem/shaderapioes2/oes2_shadow.cpp \
  materialsystem/shaderapioes2/oes2_skin.cpp \
  materialsystem/shaderapioes2/oes2_state.cpp \
  materialsystem/shaderapioes2/oes2_stencil.cpp \
  materialsystem/shaderapioes2/oes2_texture.cpp \
  materialsystem/shaderapioes2/oes2_transition.cpp
DOG_SYSTEM_LIBRARIES := EGL
ifeq ($(DOG_LOCAL_SHADERAPIOES3),true)
  DOG_C_DEFINES += SHADERAPIOES3
  DOG_SYSTEM_LIBRARIES += GLESv3
else
  DOG_SYSTEM_LIBRARIES += GLESv2
endif