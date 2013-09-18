LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := "$(DOG_SRC_MAIN)/common" "$(DOG_SRC_MAIN)/public" $(DOG_C_INCLUDES)
LOCAL_CFLAGS := -D_D0G_PACKAGE_NAME=\"$(DOG_PACKAGE_NAME)\" -D_LINUX -DWCHAR_MAX=(65535) -DWCHAR_MIN=(0) \
  -fexceptions -ffast-math -fshort-wchar -fpermissive -frtti -O2 -Wno-attributes -Wno-write-strings $(DOG_CFLAGS)
LOCAL_MODULE := $(DOG_PROJECT)_android_$(DOG_LIBSUFFIX)
LOCAL_SRC_FILES := $(DOG_SRC_FILES)
ifeq ($(DOG_STATIC_LIBRARY),true)
  LOCAL_CFLAGS += -D_LIB -D_STATIC_LINKED
  include $(BUILD_STATIC_LIBRARY)
else
  LOCAL_CFLAGS += -D_SHARED_LIB -D_USRDLL
  LOCAL_LDFLAGS := -shared -W,-( -L"$(DOG_SRC_MAIN)/bin" -L"$(DOG_SRC_MAIN)/lib" $(addprefix -l,$(DOG_SYSTEM_LIBRARIES)) $(addprefix -l,$(addsuffix _android_$(DOG_LIBSUFFIX),$(DOG_PUBLIC_LIBRARIES)))
  include $(BUILD_SHARED_LIBRARY)
endif