DOG_C_DEFINES := \
  DONT_PROTECT_FILEIO_FUNCTIONS \
  FILESYSTEM_STDIO_EXPORTS \
  PROTECTED_THINGS_ENABLE
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  tier0 \
  tier1 \
  tier2 \
  vstdlib
DOG_SRC_FILES := \
  filesystem/basefilesystem.cpp \
  filesystem/filesystem_async.cpp \
  filesystem/filesystem_stdio.cpp \
  filesystem/filetracker.cpp \
  public/kevvaluescompiler.cpp \
  public/tier0/memoverride.cpp \
  filesystem/QueuedLoader.cpp \
  public/zip_utils.cpp