LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := "$(DOG_SRC_MAIN)/common" "$(DOG_SRC_MAIN)/public" $(DOG_C_INCLUDES)
LOCAL_CFLAGS := -D_LINUX -fexceptions -fpermissive -O2 -Wno-attributes $(DOG_CFLAGS)
LOCAL_SRC_FILES := $(DOG_SRC_FILES)
ifeq ($(DOG_STATIC_LIBRARY), true)
	LOCAL_CFLAGS += -D_STATIC_LINKED
	include $(BUILD_STATIC_LIBRARY)
else
	LOCAL_CFLAGS += -D_SHARED_LIB
	LOCAL_LDFLAGS := -L"$(DOG_SRC_MAIN)/lib/$(TARGET_PLATFORM)/$(TARGET_ARCH_ABI)"
	LOCAL_LDLIBS := $(addprefix -l,$(DOG_LDLIBS))
	ifeq ($(LOCAL_ARM_NEON), true)
		LOCAL_STATIC_LIBRARIES := $(addsuffix _neon,$(DOG_STATIC_LIBRARIES))
	else
		LOCAL_STATIC_LIBRARIES := $(DOG_STATIC_LIBRARIES)
	endif
	include $(BUILD_SHARED_LIBRARY)
endif