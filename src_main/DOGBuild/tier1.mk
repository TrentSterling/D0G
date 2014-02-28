DOG_C_DEFINES := TIER1_STATIC_LIB
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_SRC_FILES := \
  tier1/bitbuf.cpp \
  tier1/byteswap.cpp \
  tier1/characterset.cpp \
  tier1/checksum_crc.cpp \
  tier1/checksum_md5.cpp \
  tier1/commandbuffer.cpp \
  tier1/convar.cpp \
  tier1/datamanager.cpp \
  tier1/diff.cpp \
  tier1/generichash.cpp \
  tier1/interface.cpp \
  tier1/KeyValues.cpp \
  tier1/lzmaDecoder.cpp \
  tier1/lzss.cpp \
  tier1/mempool.cpp \
  tier1/memstack.cpp \
  tier1/NetAdr.cpp \
  tier1/newbitbuf.cpp \
  tier1/processor_detect.cpp \
  tier1/rangecheckedvar.cpp \
  tier1/stringpool.cpp \
  tier1/strtools.cpp \
  tier1/tier1.cpp \
  tier1/tokenreader.cpp \
  tier1/undiff.cpp \
  tier1/uniqueid.cpp \
  tier1/utlbuffer.cpp \
  tier1/utlbufferutil.cpp \
  tier1/utlstring.cpp \
  tier1/utlsymbol.cpp
DOG_STATIC_LIBRARY := true