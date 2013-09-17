DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_CFLAGS := -DTIER1_STATIC_LIB
DOG_SRC_FILES := \
  bitbuf.cpp \
  byteswap.cpp \
  characterset.cpp \
  checksum_crc.cpp \
  checksum_md5.cpp \
  commandbuffer.cpp \
  convar.cpp \
  datamanager.cpp \
  diff.cpp \
  generichash.cpp \
  interface.cpp \
  KeyValues.cpp \
  lzmaDecoder.cpp \
  lzss.cpp \
  mempool.cpp \
  memstack.cpp \
  NetAdr.cpp \
  newbitbuf.cpp \
  processor_detect.cpp \
  rangecheckedvar.cpp \
  stringpool.cpp \
  strtools.cpp \
  tier1.cpp \
  tokenreader.cpp \
  undiff.cpp \
  uniqueid.cpp \
  utlbuffer.cpp \
  utlbufferutil.cpp \
  utlstring.cpp \
  utlsymbol.cpp
DOG_STATIC_LIBRARY := true