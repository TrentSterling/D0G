DOG_SRC_MAIN := $(call my-dir)
LOCAL_PATH := $(DOG_SRC_MAIN)

include DOGBuild/$(DOG_PROJECT).mk

ifneq ($(DOG_NO_CSTD),true)
  DOG_PUBLIC_LIBRARIES := cstd $(DOG_PUBLIC_LIBRARIES)
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)

  include $(CLEAR_VARS)
  DOG_LIBSUFFIX := arm
  include DOGBuildCore.mk

  ifneq ($(DOG_PROJECT),android_launcher_main)
    include $(CLEAR_VARS)
    DOG_LIBSUFFIX := neon
    LOCAL_ARM_NEON := true
    include DOGBuildCore.mk
  endif

else

  include $(CLEAR_VARS)
  DOG_LIBSUFFIX := x86
  include DOGBuildCore.mk

endif