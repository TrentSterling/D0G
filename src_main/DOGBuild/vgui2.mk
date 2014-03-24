DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  tier0 \
  tier1 \
  tier2 \
  tier3 \
  vgui_surfacelib \
  vstdlib
DOG_SRC_FILES := \
  vgui2/src/Bitmap.cpp \
  vgui2/src/Border.cpp \
  vgui2/src/fileimage.cpp \
  public/filesystem_helpers.cpp \
  public/filesystem_init.cpp \
  vgui2/src/InputWin32.cpp \
  vgui2/src/LocalizedStringTable.cpp \
  vgui2/src/MemoryBitmap.cpp \
  public/tier0/memoverride.cpp \
  vgui2/src/MessageListener.cpp \
  vgui2/src/Scheme.cpp \
  vgui2/src/System.cpp \
  public/UnicodeFileHelpers.cpp \
  vgui2/src/vgui.cpp \
  vgui2/src/vgui_internal.cpp \
  vgui2/src/vgui_key_translation.cpp \
  vgui2/src/VPanel.cpp \
  vgui2/src/VPanelWrapper.cpp