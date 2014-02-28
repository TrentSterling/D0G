DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_SRC_FILES := \
  vtf/convert_x360.cpp \
  vtf/s3tc_decode.cpp \
  vtf/vtf.cpp
DOG_STATIC_LIBRARY := true