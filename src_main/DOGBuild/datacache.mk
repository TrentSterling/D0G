DOG_C_DEFINES := MDLCACHE_DLL_EXPORT
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  tier0 \
  tier1 \
  tier2 \
  tier3 \
  vstdlib
DOG_SRC_FILES := \
  datacache/datacache.cpp \
  datacache/mdlcache.cpp \
  public/tier0/memoverride.cpp \
  public/studio.cpp \
  public/studio_virtualmodel.cpp \
  common/studiobyteswap.cpp