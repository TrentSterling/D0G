DOG_C_DEFINES := LAUNCHERONLY
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  appframework \
  tier0 \
  tier1 \
  tier2 \
  tier3 \
  vstdlib
DOG_SRC_FILES := \
  ../public/filesystem_init.cpp \
  launcher.cpp \
  ../public/tier0/memoverride.cpp