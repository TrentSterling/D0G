LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := \
  "$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/$(TOOLCHAIN_VERSION)/include" \
  "$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/$(TOOLCHAIN_VERSION)/libs/$(TARGET_ARCH_ABI)/include" \
  "$(DOG_SRC_MAIN)/common" "$(DOG_SRC_MAIN)/public" $(DOG_C_INCLUDES)
LOCAL_CFLAGS := -D_D0G_PACKAGE_NAME=\"$(DOG_PACKAGE_NAME)\" -D_LINUX=(1) -DWCHAR_MAX=(65535) -DWCHAR_MIN=(0) \
  -fexceptions -ffast-math -fpermissive -frtti -fshort-wchar -O2 \
  -Wno-attributes -Wno-invalid-offsetof -Wno-write-strings $(DOG_CFLAGS)
LOCAL_MODULE := $(DOG_PROJECT)_android_$(DOG_LIBSUFFIX)
LOCAL_SRC_FILES := $(DOG_SRC_FILES)
ifeq ($(DOG_STATIC_LIBRARY),true)
  LOCAL_CFLAGS += -D_LIB -D_STATIC_LINKED
  include $(BUILD_STATIC_LIBRARY)
else
  LOCAL_CFLAGS += -D_SHARED_LIB -D_USRDLL
  DOG_LIBRARIES_PREFIXED := $(addprefix -l,$(DOG_SYSTEM_LIBRARIES) $(addsuffix _android_$(DOG_LIBSUFFIX),$(DOG_PUBLIC_LIBRARIES)))
  LOCAL_LDFLAGS := -shared \
    -L"$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/$(TOOLCHAIN_VERSION)/libs/$(TARGET_ARCH_ABI)" \
    -L"$(DOG_SRC_MAIN)/bin" -L"$(DOG_SRC_MAIN)/lib" \
    -lgnustl_shared -Lsupc++ $(DOG_LIBRARIES_PREFIXED) $(DOG_LIBRARIES_PREFIXED)
  include $(BUILD_SHARED_LIBRARY)
endif