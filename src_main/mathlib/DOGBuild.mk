DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1" \
  "$(DOG_SRC_MAIN)/public/mathlib"
DOG_SRC_FILES := \
  3dnow.cpp \
  anorms.cpp \
  bumpvects.cpp \
  color_conversion.cpp \
  halton.cpp \
  IceKey.cpp \
  imagequant.cpp \
  lightdesc.cpp \
  mathlib_base.cpp \
  polyhedron.cpp \
  powsse.cpp \
  quantize.cpp \
  randsse.cpp \
  simdvectormatrix.cpp \
  sparse_convolution_noise.cpp \
  sse.cpp \
  sseconst.cpp \
  ssenoise.cpp \
  vector.cpp \
  vmatrix.cpp
DOG_STATIC_LIBRARY := true