DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_SRC_FILES := \
  bitmap/colorconversion.cpp \
  bitmap/float_bm.cpp \
  bitmap/float_bm2.cpp \
  bitmap/float_bm3.cpp \
  bitmap/float_bm4.cpp \
  bitmap/float_bm_bilateral_filter.cpp \
  bitmap/float_cube.cpp \
  bitmap/ImageByteSwap.cpp \
  bitmap/imageformat.cpp \
  bitmap/psd.cpp \
  bitmap/resample.cpp \
  bitmap/tgaloader.cpp \
  bitmap/tgawriter.cpp
DOG_STATIC_LIBRARY := true