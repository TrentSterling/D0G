DOG_C_DEFINES := VSTDLIB_DLL_EXPORT
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  tier0 \
  tier1
DOG_SRC_FILES := \
  vstdlib/cvar.cpp \
  vstdlib/jobthread.cpp \
  vstdlib/KeyValuesSystem.cpp \
  public/tier0/memoverride.cpp \
  vstdlib/processutils.cpp \
  vstdlib/random.cpp \
  vstdlib/vcover.cpp