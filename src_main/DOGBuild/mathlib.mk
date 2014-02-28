DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1" \
  "$(DOG_SRC_MAIN)/public/mathlib"
DOG_SRC_FILES := \
  mathlib/3dnow.cpp \
  mathlib/anorms.cpp \
  mathlib/bumpvects.cpp \
  mathlib/color_conversion.cpp \
  mathlib/halton.cpp \
  mathlib/IceKey.cpp \
  mathlib/imagequant.cpp \
  mathlib/lightdesc.cpp \
  mathlib/mathlib_base.cpp \
  mathlib/polyhedron.cpp \
  mathlib/powsse.cpp \
  mathlib/quantize.cpp \
  mathlib/randsse.cpp \
  mathlib/simdvectormatrix.cpp \
  mathlib/sparse_convolution_noise.cpp \
  mathlib/sse.cpp \
  mathlib/sseconst.cpp \
  mathlib/ssenoise.cpp \
  mathlib/vector.cpp \
  mathlib/vmatrix.cpp
DOG_STATIC_LIBRARY := true