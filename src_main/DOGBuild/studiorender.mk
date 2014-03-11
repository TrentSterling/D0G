DOG_C_DEFINES := STUDIORENDER_EXPORTS
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  bitmap \
  mathlib \
  tier0 \
  tier1 \
  tier2 \
  tier3 \
  vstdlib
DOG_SRC_FILES := \
  studiorender/flexrenderdata.cpp \
  public/tier0/memoverride.cpp \
  studiorender/r_studio.cpp \
  studiorender/r_studiodecal.cpp \
  studiorender/r_studiodraw.cpp \
  studiorender/r_studioflex.cpp \
  studiorender/r_studiogettriangles.cpp \
  studiorender/r_studiolight.cpp \
  studiorender/studiorender.cpp \
  studiorender/studiorendercontext.cpp