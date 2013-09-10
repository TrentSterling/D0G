LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := "$(DOG_SRC_MAIN)/common" "$(DOG_SRC_MAIN)/public" $(DOG_C_INCLUDES)
LOCAL_CFLAGS := -D_LINUX -fexceptions -fshort-wchar -fpermissive -O2 $(DOG_CFLAGS)
LOCAL_SRC_FILES := $(DOG_SRC_FILES)
ifeq ($(DOG_STATIC_LIBRARY),true)
  LOCAL_CFLAGS += -D_STATIC_LINKED
  include $(BUILD_STATIC_LIBRARY)
else
  LOCAL_CFLAGS += -D_SHARED_LIB
  LOCAL_LDFLAGS := -shared -W,-( -L"$(DOG_SRC_MAIN)/bin/android/$(TARGET_ARCH_ABI)" -L"$(DOG_SRC_MAIN)/lib/android/$(TARGET_ARCH_ABI)" $(addprefix -l,$(DOG_SYSTEM_LIBRARIES))
  ifeq ($(LOCAL_ARM_NEON),true)
    LOCAL_LDFLAGS += $(addprefix -l,$(addsuffix _neon,$(DOG_PUBLIC_LIBRARIES)))
  else
    LOCAL_LDFLAGS += $(addprefix -l,$(DOG_PUBLIC_LIBRARIES))
  endif
  include $(BUILD_SHARED_LIBRARY)
endif