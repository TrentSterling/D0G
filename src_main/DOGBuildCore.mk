LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := "$(DOG_SRC_MAIN)/common" "$(DOG_SRC_MAIN)/public" $(DOG_C_INCLUDES)
LOCAL_CFLAGS := -D_LINUX -fexceptions -fpermissive -O2 $(DOG_CFLAGS)
LOCAL_SRC_FILES := $(DOG_SRC_FILES)
ifeq ($(DOG_STATIC_LIBRARY), true)
	LOCAL_CFLAGS += -D_STATIC_LINKED
	include $(BUILD_STATIC_LIBRARY)
else
	LOCAL_CFLAGS += -D_SHARED_LIB
	LOCAL_LDFLAGS := -L"$(DOG_SRC_MAIN)/lib/android/$(TARGET_ARCH_ABI)" $(DOG_LDFLAGS)
	LOCAL_LDLIBS := $(DOG_LDLIBS)
	ifeq ($(LOCAL_ARM_NEON), true)
		LOCAL_LDLIBS += $(addsuffix _neon,$(DOG_LDLIBS_PUBLIC))
	else
		LOCAL_LDLIBS += $(DOG_LDLIBS_PUBLIC)
	endif
	include $(BUILD_SHARED_LIBRARY)
endif