DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1" \
  "$(DOG_SRC_MAIN)/public/tier2"
DOG_SRC_FILES := \
  tier2/beamsegdraw.cpp \
  tier2/defaultfilesystem.cpp \
  tier2/dmconnect.cpp \
  tier2/fileutils.cpp \
  tier2/keybindings.cpp \
  public/map_utils.cpp \
  public/materialsystem/MaterialSystemUtil.cpp \
  tier2/meshutils.cpp \
  tier2/p4helpers.cpp \
  tier2/renderutils.cpp \
  tier2/riff.cpp \
  tier2/soundutils.cpp \
  tier2/tier2.cpp \
  tier2/util_init.cpp \
  tier2/utlstreambuffer.cpp \
  tier2/vconfig.cpp
DOG_STATIC_LIBRARY := true