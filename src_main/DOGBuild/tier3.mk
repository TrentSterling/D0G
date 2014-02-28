DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_SRC_FILES := \
  tier3/choreoutils.cpp \
  tier3/scenetokenprocessor.cpp \
  tier3/studiohdrstub.cpp \
  tier3/tier3.cpp
DOG_STATIC_LIBRARY := true