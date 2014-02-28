DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_SRC_FILES := \
  convert_x360.cpp \
  s3tc_decode.cpp \
  vtf.cpp
DOG_STATIC_LIBRARY := true