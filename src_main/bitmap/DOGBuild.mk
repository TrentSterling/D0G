DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_SRC_FILES := \
  colorconversion.cpp \
  float_bm.cpp \
  float_bm2.cpp \
  float_bm3.cpp \
  float_bm4.cpp \
  float_bm_bilateral_filter.cpp \
  float_cube.cpp \
  ImageByteSwap.cpp \
  imageformat.cpp \
  psd.cpp \
  resample.cpp \
  tgaloader.cpp \
  tgawriter.cpp
DOG_STATIC_LIBRARY := true