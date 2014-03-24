DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/common/freetype" \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_SRC_FILES := \
  vgui2/vgui_surfacelib/BitmapFont.cpp \
  vgui2/vgui_surfacelib/FontAmalgam.cpp \
  vgui2/vgui_surfacelib/FontEffects.cpp \
  vgui2/vgui_surfacelib/FontManager.cpp \
  vgui2/vgui_surfacelib/Win32Font_android.cpp
DOG_STATIC_LIBRARY := true