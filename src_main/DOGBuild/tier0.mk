DOG_C_DEFINES := \
  TIER0_DLL_EXPORT
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_SRC_FILES := \
  tier0/android_system.cpp \
  tier0/assert_dialog.cpp \
  tier0/commandline.cpp \
  tier0/cpu.cpp \
  tier0/dbg.cpp \
  tier0/extendedtrace.cpp \
  tier0/fasttimer.cpp \
  tier0/mem.cpp \
  tier0/memdbg.cpp \
  tier0/meminit.cpp \
  tier0/memstd.cpp \
  tier0/memvalidate.cpp \
  tier0/mem_helpers.cpp \
  tier0/minidump.cpp \
  tier0/pch_tier0.cpp \
  tier0/platform.cpp \
  tier0/pme.cpp \
  tier0/progressbar.cpp \
  tier0/security.cpp \
  tier0/systeminformation.cpp \
  tier0/thread.cpp \
  tier0/threadtools.cpp \
  tier0/tslist.cpp \
  tier0/validator.cpp \
  tier0/valobject.cpp \
  tier0/vcrmode.cpp \
  tier0/vprof.cpp \
  tier0/win32consoleio.cpp