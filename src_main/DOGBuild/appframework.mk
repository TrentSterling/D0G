DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_SRC_FILES := \
  appframework/AppSystemGroup.cpp \
  public/filesystem_init.cpp \
  appframework/WinApp.cpp
DOG_STATIC_LIBRARY := true