LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := \
  "$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/$(TOOLCHAIN_VERSION)/include" \
  "$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/$(TOOLCHAIN_VERSION)/libs/$(TARGET_ARCH_ABI)/include" \
  "$(DOG_SRC_MAIN)/common" "$(DOG_SRC_MAIN)/public" $(DOG_C_INCLUDES)
LOCAL_CFLAGS := -D_D0G_PACKAGE_NAME=\"$(DOG_PACKAGE_NAME)\" -D_LINUX=(1) -ffast-math -fpermissive 
ifneq ($(DOG_LONG_WCHAR),true)
  LOCAL_CFLAGS += -DWCHAR_MAX=(65535) -DWCHAR_MIN=(0) -fshort-wchar
endif
ifneq ($(DOG_PROJECT),srcactivity)
  LOCAL_CFLAGS += -fexceptions -frtti
endif
LOCAL_CFLAGS += -O2 -Wno-attributes -Wno-invalid-offsetof -Wno-write-strings $(DOG_CFLAGS)
ifeq ($(DOG_PROJECT),srcactivity)
  LOCAL_MODULE := srcactivity
else
  LOCAL_MODULE := $(DOG_PROJECT)_android_$(DOG_LIBSUFFIX)
endif
LOCAL_SRC_FILES := $(DOG_SRC_FILES)
ifeq ($(DOG_STATIC_LIBRARY),true)
  LOCAL_CFLAGS += -D_LIB -D_STATIC_LINKED
  include $(BUILD_STATIC_LIBRARY)
else
  LOCAL_CFLAGS += -D_SHARED_LIB -D_USRDLL
  LOCAL_LDFLAGS := -shared
  ifneq ($(DOG_PROJECT),srcactivity)
    LOCAL_LDFLAGS += -L"$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/$(TOOLCHAIN_VERSION)/libs/$(TARGET_ARCH_ABI)" -lgnustl_shared -lsupc++
  endif
  DOG_LIBRARIES_PREFIXED := $(addprefix -l,$(DOG_SYSTEM_LIBRARIES) $(addsuffix _android_$(DOG_LIBSUFFIX),$(DOG_PUBLIC_LIBRARIES)))
  LOCAL_LDFLAGS += -L"$(DOG_SRC_MAIN)/bin" -L"$(DOG_SRC_MAIN)/lib" $(DOG_LIBRARIES_PREFIXED) $(DOG_LIBRARIES_PREFIXED)
  include $(BUILD_SHARED_LIBRARY)
endif