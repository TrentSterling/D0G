DOG_C_DEFINES := \
  BENCHMARK \
  VGUIMATSURFACE_DLL_EXPORT \
  GAMEUI_EXPORTS
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/common/freetype" \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  bitmap \
  freetype \
  mathlib \
  tier0 \
  tier1 \
  tier2 \
  tier3 \
  vgui_controls \
  vgui_surfacelib \
  vstdlib
DOG_SRC_FILES := \
  vguimatsurface/Clip2D.cpp \
  public/filesystem_helpers.cpp \
  vguimatsurface/FontTextureCache.cpp \
  vguimatsurface/Input.cpp \
  vguimatsurface/MatSystemSurface.cpp \
  vguimatsurface/memorybitmap.cpp \
  public/tier0/memoverride.cpp \
  vguimatsurface/TextureDictionary.cpp \
  public/vgui_controls/vgui_controls.cpp \
  vgui2/src/vgui_key_translation.cpp