DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1" \
  "$(DOG_SRC_MAIN)/public/tier2"
DOG_SRC_FILES := \
  beamsegdraw.cpp \
  defaultfilesystem.cpp \
  dmconnect.cpp \
  fileutils.cpp \
  keybindings.cpp \
  ../public/map_utils.cpp \
  ../public/materialsystem/MaterialSystemUtil.cpp \
  meshutils.cpp \
  p4helpers.cpp \
  renderutils.cpp \
  riff.cpp \
  soundutils.cpp \
  tier2.cpp \
  util_init.cpp \
  utlstreambuffer.cpp \
  vconfig.cpp
DOG_STATIC_LIBRARY := true