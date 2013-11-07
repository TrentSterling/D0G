LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := \
  "$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/$(TOOLCHAIN_VERSION)/include" \
  "$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/$(TOOLCHAIN_VERSION)/libs/$(TARGET_ARCH_ABI)/include" \
  "$(DOG_SRC_MAIN)/common" "$(DOG_SRC_MAIN)/public" $(DOG_C_INCLUDES)
LOCAL_CFLAGS := -DNDEBUG -D_LINUX=1 $(addprefix -D,$(DOG_C_DEFINES)) -ffast-math -O2 -Wno-attributes -Wno-write-strings
ifneq ($(DOG_LONG_WCHAR),true)
  LOCAL_CFLAGS += -DWCHAR_MAX=(65535) -DWCHAR_MIN=(0) -fshort-wchar
endif
LOCAL_CPPFLAGS := -fpermissive -Wno-invalid-offsetof
ifneq ($(DOG_PROJECT),android_launcher_main)
  LOCAL_CPPFLAGS += -fexceptions -frtti
endif
LOCAL_CFLAGS += $(DOG_CFLAGS)
LOCAL_CPPFLAGS += $(DOG_CPPFLAGS)
ifeq ($(DOG_PROJECT),android_launcher_main)
  LOCAL_MODULE := android_launcher_main
else
  LOCAL_MODULE := $(notdir $(DOG_PROJECT))_android_$(DOG_LIBSUFFIX)
endif
LOCAL_SHORT_COMMANDS := true
LOCAL_SRC_FILES := $(DOG_SRC_FILES)
ifeq ($(DOG_STATIC_LIBRARY),true)
  LOCAL_CFLAGS += -D_LIB -D_STATIC_LINKED
  include $(BUILD_STATIC_LIBRARY)
else
  LOCAL_CFLAGS += -D_SHARED_LIB -D_USRDLL
  LOCAL_LDFLAGS := -shared -landroid -llog
  ifneq ($(DOG_PROJECT),android_launcher_main)
    LOCAL_LDFLAGS += -L"$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/$(TOOLCHAIN_VERSION)/libs/$(TARGET_ARCH_ABI)" -lgnustl_shared -lsupc++
  endif
  DOG_LIBRARIES_PREFIXED := $(addprefix -l,$(DOG_SYSTEM_LIBRARIES) $(addsuffix _android_$(DOG_LIBSUFFIX),$(DOG_PUBLIC_LIBRARIES)))
  LOCAL_LDFLAGS += -L"$(DOG_SRC_MAIN)/bin" -L"$(DOG_SRC_MAIN)/lib" $(DOG_LIBRARIES_PREFIXED) $(DOG_LIBRARIES_PREFIXED)
  include $(BUILD_SHARED_LIBRARY)
endif