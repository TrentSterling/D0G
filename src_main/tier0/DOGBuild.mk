DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_CFLAGS := -DTIER0_DLL_EXPORT
DOG_PUBLIC_LIBRARIES := \
  cstd
DOG_SRC_FILES := \
  assert_dialog.cpp \
  commandline.cpp \
  cpu.cpp \
  dbg.cpp \
  extendedtrace.cpp \
  fasttimer.cpp \
  mem.cpp \
  memdbg.cpp \
  meminit.cpp \
  memstd.cpp \
  memvalidate.cpp \
  mem_helpers.cpp \
  minidump.cpp \
  pch_tier0.cpp \
  platform.cpp \
  pme.cpp \
  progressbar.cpp \
  security.cpp \
  systeminformation.cpp \
  thread.cpp \
  threadtools.cpp \
  tslist.cpp \
  validator.cpp \
  valobject.cpp \
  vcrmode.cpp \
  vprof.cpp \
  win32consoleio.cpp